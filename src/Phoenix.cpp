#include "plugin.hpp"


struct BaselineTracker {
	float current = 1.f; float target = 1.f; float startValue = 1.f;
	float progress = 0.f;

	float linExpRatio = 0.f;

	float MIN_RECOVERY_SPEED = 0.01f;
	float recoverySpeed = MIN_RECOVERY_SPEED;

	enum State { IDLE, RECOVERING, RECOVERED } state = IDLE;

	enum RunningMode { NORMAL, INVERTED } mode = NORMAL;
	enum WeakeningMode { ALWAYS, UNTIL_RECOVERED } weakeningMode = ALWAYS;

	float process(const float delta) {
		if (current == target) {
			state = IDLE;
			return current;
		}

		progress += delta;
		float p = clamp(progress / recoverySpeed, 0.f, 1.f);

		const float linP = p;
		const float expP = easeInAndOut(p);
		p = crossfade(linP, expP, linExpRatio);

		current = crossfade(startValue, target, p);

		if (p >= 1.f) {
			current = target;
			progress = 0.f;
			startValue = current;
		}

		state = current == target ? RECOVERED : RECOVERING;

		return current;
	}

	float getCurrent() const {
		return current;
	}

	bool weaken(const float strength) {
		if (weakeningMode == UNTIL_RECOVERED && state == RECOVERING) {
			return false;
		}

		current = mode == NORMAL ? current - strength : current + strength;
		current = clamp(current, 0.f, 1.f);
		startValue = current;
		progress = 0.f;

		const bool hasFallen = mode == NORMAL ? current == 0.f : current == 1.f;
		return state != RECOVERING && hasFallen;
	}

	RunningMode invert() {
		mode = mode == NORMAL ? INVERTED : NORMAL;
		target = mode == NORMAL ? 1.f : 0.f;
		startValue = current;
		progress = 0.f;
		return mode;
	}

	State getState() const {
		return state;
	}

	void setRecoverySpeed(float speed) {
		recoverySpeed = std::max(speed, MIN_RECOVERY_SPEED);
	}

	static float easeInAndOut(const float p) {
		if (p < 0.5) {
			return 4 * p * p * p;
		}

		return (p - 1) * (2 * p - 2) * (2 * p - 2) + 1;
	}

	void setLinExpRatio(const float val) {
		linExpRatio = val;
	}
};


struct Phoenix final : Module {
	enum ParamId {
		RISE_PARAM,
		FALL_PARAM,
		RISE_CV_PARAM,
		FALL_CV_PARAM,
		OM_PARAM,
		AM_PARAM,
		WM_PARAM,
		LIN_EXP_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		RISE_INPUT,
		FALL_INPUT,
		INVERT_INPUT,
		MAIN_INPUT,
		HIT_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		RISEN_OUTPUT,
		FALLEN_OUTPUT,
		AUX_OUTPUT,
		MAIN_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(OM_LIGHT, 3),
		ENUMS(AM_LIGHT, 3),
		ENUMS(WM_LIGHT, 3),
		LIGHTS_LEN
	};

	// TODO: Save / Load these
	BaselineTracker baseline;

	dsp::SchmittTrigger weakenTrigger;
	dsp::SchmittTrigger invertTrigger;
	dsp::PulseGenerator risen;
	dsp::PulseGenerator fallen;

	dsp::BooleanTrigger omTrigger;
	dsp::BooleanTrigger amTrigger;
	dsp::BooleanTrigger wmTrigger;

	// TODO: SIGNAL -> MAIN
	// TODO: OUT -> MAIN

	float RISE_PARAM_MIN = 0.f;
	float RISE_PARAM_MAX = 13.f;

	float FALL_PARAM_MIN = 0.f;
	float FALL_PARAM_MAX = 1.f;

	enum AttenuationMode { ATTENUATION, NUDGE } attenuationMode = NUDGE;

	Phoenix() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(RISE_PARAM, RISE_PARAM_MIN, RISE_PARAM_MAX, 0.1f, "Rise", " s");
		configParam(FALL_PARAM, FALL_PARAM_MIN, FALL_PARAM_MAX, 0.1f, "Hit strength");
		configParam(RISE_CV_PARAM, -1.f, 1.f, 0.f, "Rise CV");
		configParam(FALL_CV_PARAM, -1.f, 1.f, 0.f, "Fall CV");
		configButton(OM_PARAM, "Output range"); // TODO: Rename these.
		configButton(AM_PARAM, "Attenuation mode");
		configButton(WM_PARAM, "Hit behavior");
		configParam(LIN_EXP_PARAM, 0.f, 1.f, 0.f, "Linear / Exponential rise");
		configInput(RISE_INPUT, "Rise CV (-5V/5V)");
		configInput(FALL_INPUT, "Fall CV (-5V/5V)");
		configInput(INVERT_INPUT, "Invert trigger");
		configInput(MAIN_INPUT, "Main");
		configInput(HIT_INPUT, "Hit");
		configOutput(RISEN_OUTPUT, "Risen");
		configOutput(FALLEN_OUTPUT, "Fallen");
		configOutput(AUX_OUTPUT, "AUX (-5V/5V)");
		configOutput(MAIN_OUTPUT, "Main");
	}

	void process(const ProcessArgs& args) override {
		baseline.setLinExpRatio(getParam(LIN_EXP_PARAM).getValue());

		// Handle attenuation mode.
		if (amTrigger.process(getParam(AM_PARAM).getValue())) {
			attenuationMode = attenuationMode == ATTENUATION ? NUDGE : ATTENUATION;
		}

		const float recoverySpeed = getAttenuverted(RISE_PARAM, RISE_INPUT, RISE_CV_PARAM, RISE_PARAM_MIN, RISE_PARAM_MAX);
		baseline.setRecoverySpeed(recoverySpeed);

		if (invertTrigger.process(getInput(INVERT_INPUT).getVoltage())) {
			baseline.invert();
		}

		bool hasFallen = false;
		if (weakenTrigger.process(getInput(HIT_INPUT).getVoltage(), 0.1f, 2.f)) {
			hasFallen = baseline.weaken(getAttenuverted(FALL_PARAM, FALL_INPUT, FALL_CV_PARAM, FALL_PARAM_MIN, FALL_PARAM_MAX));
		}

		baseline.process(args.sampleTime);

		// TODO: Polyphony.
		// TODO: Configurable input/output ranges (e.g. -5V to 5V, 0V to 10V, etc.)
		const float signal = getInput(MAIN_INPUT).getVoltage();

		float out = 0.f;
		switch (attenuationMode) {
			case ATTENUATION:
				out = signal * baseline.getCurrent();
				break;
			case NUDGE:
				const float y_max = rescale(baseline.getCurrent(), 0.f, 1.f, -10.f, 10.f);
				out = rescale(signal, -10.f, 10.f, -10.f, y_max);
				break;
		}

		if (baseline.getState() == BaselineTracker::RECOVERED) {
			risen.trigger();
		}
		getOutput(RISEN_OUTPUT).setVoltage(risen.process(args.sampleTime) ? 10.f : 0.f);

		if (hasFallen) {
			fallen.trigger();
		}
		getOutput(FALLEN_OUTPUT).setVoltage(fallen.process(args.sampleTime) ? 10.f : 0.f);

		const float aux = rescale(baseline.getCurrent(), 0.f, 1.f, -5.f, 5.f);
		getOutput(AUX_OUTPUT).setVoltage(aux);
		getOutput(MAIN_OUTPUT).setVoltage(out);

		setLight(AM_LIGHT, 1.f, attenuationMode == ATTENUATION ? GREEN : BLUE, args.sampleTime);
	}

	enum Color { GREEN, BLUE };

	void setLight(const LightId lightId, const float brightness, const Color color, float delta) {
		float r = 0.f, g = 0.f, b = 0.f;

		switch (color) {
		case GREEN:
			r = 0.f;
			g = brightness;
			b = 0.f;
			break;
		case BLUE:
			r = 0.f;
			g = 0.f;
			b = brightness;
			break;
		}

		getLight(lightId + 0).setBrightnessSmooth(r, delta);
		getLight(lightId + 1).setBrightnessSmooth(g, delta);
		getLight(lightId + 2).setBrightnessSmooth(b, delta);
	}

	float getAttenuverted(const ParamId paramId, const InputId inputId, const ParamId attParamId, const float paramMin, const float paramMax) {
		const float param = getParam(paramId).getValue();
		if (!getInput(inputId).isConnected()) {
			return param;
		}

		const float att = getParam(attParamId).getValue();
		float in = getInput(inputId).getVoltage();
		in = rescale(in, -5.f, 5.f, paramMin, paramMax);

		return clamp(param + in * att, paramMin, paramMax);
	}
};


struct PhoenixWidget final : ModuleWidget {
	explicit PhoenixWidget(Phoenix* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Phoenix.svg")));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(8.25, 17.75)), module, Phoenix::RISE_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(22.25, 17.75)), module, Phoenix::FALL_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(8.25, 26.009)), module, Phoenix::RISE_CV_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(22.25, 26.009)), module, Phoenix::FALL_CV_PARAM));
		addParam(createLightParamCentered<VCVLightButton<MediumSimpleLight<RedGreenBlueLight>>>(mm2px(Vec(6.491, 49.037)), module, Phoenix::OM_PARAM, Phoenix::OM_LIGHT));
		addParam(createLightParamCentered<VCVLightButton<MediumSimpleLight<RedGreenBlueLight>>>(mm2px(Vec(15.24, 49.037)), module, Phoenix::AM_PARAM, Phoenix::AM_LIGHT));
		addParam(createLightParamCentered<VCVLightButton<MediumSimpleLight<RedGreenBlueLight>>>(mm2px(Vec(23.989, 49.037)), module, Phoenix::WM_PARAM, Phoenix::WM_LIGHT));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(22.25, 64.25)), module, Phoenix::LIN_EXP_PARAM));

		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(8.25, 34.269)), module, Phoenix::RISE_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(22.25, 34.273)), module, Phoenix::FALL_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(8.51, 64.235)), module, Phoenix::INVERT_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(8.25, 97.25)), module, Phoenix::MAIN_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(22.25, 97.25)), module, Phoenix::HIT_INPUT));

		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(8.25, 80.75)), module, Phoenix::RISEN_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(22.25, 80.75)), module, Phoenix::FALLEN_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(8.25, 113.75)), module, Phoenix::AUX_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(22.25, 113.75)), module, Phoenix::MAIN_OUTPUT));
	}
};


Model* modelPhoenix = createModel<Phoenix, PhoenixWidget>("Phoenix");
