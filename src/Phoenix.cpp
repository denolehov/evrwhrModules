#include "plugin.hpp"


struct BaselineTracker {
	float current = 1.f; float target = 1.f; float startValue = 1.f;
	float progress = 0.f;

	float MIN_RECOVERY_SPEED = 0.01f;
	float recoverySpeed = MIN_RECOVERY_SPEED;

	enum State { IDLE, RECOVERING, RECOVERED } state = IDLE;

	enum RunningMode { NORMAL, INVERTED } mode = NORMAL;
	enum WeakeningMode { ALWAYS, UNTIL_RECOVERED } weakeningMode = UNTIL_RECOVERED;

	float process(const float delta) {
		if (current == target) {
			state = IDLE;
			return current;
		}

		progress += delta;
		const float p = clamp(progress / recoverySpeed, 0.f, 1.f);

		current = crossfade(startValue, target, easeInAndOut(p));

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

	void weaken(const float strength) {
		if (weakeningMode == UNTIL_RECOVERED && state == RECOVERING) {
			return;
		}

		current = mode == NORMAL ? current - strength : current + strength;
		current = clamp(current, 0.f, 1.f);
		startValue = current;
		progress = 0.f;
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
	dsp::PulseGenerator eoc;

	// TODO: SIGNAL -> MAIN
	// TODO: OUT -> MAIN

	float RISE_PARAM_MIN = 0.f;
	float RISE_PARAM_MAX = 13.f;

	Phoenix() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(RISE_PARAM, RISE_PARAM_MIN, RISE_PARAM_MAX, 0.1f, "Rise", " s");
		configParam(FALL_PARAM, 0.f, 1.f, 0.1f, "Hit strength");
		configParam(RISE_CV_PARAM, -1.f, 1.f, 0.f, "Rise CV");
		configParam(FALL_CV_PARAM, 0.f, 1.f, 0.f, "Fall CV");
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
		configOutput(AUX_OUTPUT, "AUX");
		configOutput(MAIN_OUTPUT, "Main");
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

	void process(const ProcessArgs& args) override {
		float recoverySpeed = getAttenuverted(RISE_PARAM, RISE_INPUT, RISE_CV_PARAM, RISE_PARAM_MIN, RISE_PARAM_MAX);
		baseline.setRecoverySpeed(recoverySpeed);

		if (invertTrigger.process(getInput(INVERT_INPUT).getVoltage())) {
			baseline.invert();
		}

		if (weakenTrigger.process(getInput(HIT_INPUT).getVoltage(), 0.1f, 2.f)) {
			baseline.weaken(getParam(FALL_PARAM).getValue());
		}

		baseline.process(args.sampleTime);

		// TODO: Polyphony.
		// TODO: Configurable input/output ranges (e.g. -5V to 5V, 0V to 10V, etc.)
		const float signal = getInput(MAIN_INPUT).getVoltage();

		// TODO: Attenuation mode.
		// This is for attenuation mode.
		// const float out = signal * baseline.getCurrent();

		// This is for "nudge" (offset) mode.
		const float y_max = rescale(baseline.getCurrent(), 0.f, 1.f, -10.f, 10.f);
		const float out = rescale(signal, -10.f, 10.f, -10.f, y_max);

		if (baseline.getState() == BaselineTracker::RECOVERED) {
			eoc.trigger();
		}
		getOutput(FALLEN_OUTPUT).setVoltage(eoc.process(args.sampleTime) ? 10.f : 0.f);
		getOutput(MAIN_OUTPUT).setVoltage(out);
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
