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


struct Phoenix : Module {
	enum ParamId {
		FALL_PARAM,
		RECOVER_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		SIGNAL_INPUT,
		TRIGGER_INPUT,
		INVERT_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		EOC_OUTPUT,
		OUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	// TODO: Save / Load these
	BaselineTracker baseline;

	dsp::SchmittTrigger weakenTrigger;
	dsp::SchmittTrigger invertTrigger;
	dsp::PulseGenerator eoc;

	// TODO: SIGNAL -> MAIN
	// TODO: OUT -> MAIN

	Phoenix() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(FALL_PARAM, 0.f, 1.f, .1f, "");
		configParam(RECOVER_PARAM, 0.01f, 300.f, 0.01f, "Recover speed", " s");
		configInput(SIGNAL_INPUT, "");
		configInput(TRIGGER_INPUT, "");
		configInput(INVERT_INPUT, "");
		configOutput(EOC_OUTPUT, "");
		configOutput(OUT_OUTPUT, "");
	}

	void process(const ProcessArgs& args) override {
		baseline.setRecoverySpeed(getParam(RECOVER_PARAM).getValue());

		if (invertTrigger.process(getInput(INVERT_INPUT).getVoltage())) {
			baseline.invert();
		}

		if (weakenTrigger.process(getInput(TRIGGER_INPUT).getVoltage(), 0.1f, 2.f)) {
			// TODO: Add a "slow" mode where the baseline is nudged by an random amount (up to a user-defined limit).
			baseline.weaken(getParam(FALL_PARAM).getValue());
		}

		baseline.process(args.sampleTime);

		// TODO: Polyphony.
		// TODO: Configurable input/output ranges (e.g. -5V to 5V, 0V to 10V, etc.)
		const float signal = getInput(SIGNAL_INPUT).getVoltage();

		// TODO: Attenuation mode.
		// This is for attenuation mode.
		// const float out = signal * baseline.getCurrent();

		// This is for "nudge" (offset) mode.
		const float y_max = rescale(baseline.getCurrent(), 0.f, 1.f, -10.f, 10.f);
		const float out = rescale(signal, -10.f, 10.f, -10.f, y_max);

		if (baseline.getState() == BaselineTracker::RECOVERED) {
			eoc.trigger();
		}
		getOutput(EOC_OUTPUT).setVoltage(eoc.process(args.sampleTime) ? 10.f : 0.f);
		getOutput(OUT_OUTPUT).setVoltage(out);
	}
};


struct PhoenixWidget : ModuleWidget {
	PhoenixWidget(Phoenix* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Phoenix.svg")));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(7.62, 48.25)), module, Phoenix::FALL_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(7.62, 64.75)), module, Phoenix::RECOVER_PARAM));

		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 15.25)), module, Phoenix::SIGNAL_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 31.92)), module, Phoenix::TRIGGER_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 94.25)), module, Phoenix::INVERT_INPUT));

		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 80.409)), module, Phoenix::EOC_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 110.75)), module, Phoenix::OUT_OUTPUT));
	}
};


Model* modelPhoenix = createModel<Phoenix, PhoenixWidget>("Phoenix");
