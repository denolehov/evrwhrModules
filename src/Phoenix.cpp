#include "plugin.hpp"

struct BaselineTracker {
	enum State { IDLE, FALLING, RISING } state = IDLE;

	float baseline = 1.f;
	float start = 1.f, current = 1.f, target = 1.f;
	float progress = 0.f;

	float MIN_SPEED = 0.0f;
	float fallSpeed = MIN_SPEED, riseSpeed = MIN_SPEED;

	void processState() {
		// B = T = C
		if (baseline == target && target == current) {
			state = IDLE;
		}

		// B = T > C
		if (baseline == target && target > current) {
			state = RISING;
		}

		// B = T < C
		if (baseline == target && target < current) {
			state = FALLING;
		}

		// B = C > T
		if (baseline == current && current > target) {
			state = FALLING;
		}

		// B = C < T
		if (baseline == current && current < target) {
			state = RISING;
		}

		// T = C > B
		if (target == current && current > baseline) {
			target = baseline;
			start = current;
			progress = 0.f;
			state = FALLING;
		}

		// T = C < B
		if (target == current && current < baseline) {
			target = baseline;
			start = current;
			progress = 0.f;
			state = RISING;
		}

		// B > T > C
		if (baseline > target && target > current) {
			state = RISING;
		}

		// B > C > T
		if (baseline > current && current > target) {
			state = FALLING;
		}

		// T > B > C
		if (target > baseline && baseline > current) {
			target = baseline;
			progress = 0.f;
			state = RISING;
		}

		// T > C > B
		if (target > current && current > baseline) {
			state = RISING;
		}

		// C > B > T
		if (current > baseline && baseline > target) {
			target = baseline;
			progress = 0.f;
			state = FALLING;
		}

		// C > T > B
		if (current > target && target > baseline) {
			state = FALLING;
		}
	}

	float process(const float delta) {
		processState();

		if (state == IDLE) {
			return current;
		}

		progress += delta;
		const float speed = state == RISING ? riseSpeed : fallSpeed;
		float p = 1.f;
		if (speed > 0.f) {
			p = clamp(progress / speed, 0.f, 1.f);
		}

		current = crossfade(start, target, p);

		// if current is "close enough" to target, just snap it to target
		if (fabs(current - target) < 0.001f) {
			current = target;
		}

		DEBUG("STATE: %d, PROGRESS: %f, CURRENT: %f, TARGET: %f, BASELINE: %f", state, progress, current, target, baseline);

		return current;
	}

	float getCurrent() const {
		return current;
	}

	void hit(const float strength) {
		target = clamp(current - strength, 0.f, 1.f);
		progress = state == FALLING ? progress : 0.f;
		start = current;
	}

	void recover(const float strength) {
		target = clamp(current + strength, 0.f, 1.f);
		progress = state == RISING ? progress : 0.f;
		start = current;
	}

	void setRiseSpeed(const float value) {
		riseSpeed = std::max(value, MIN_SPEED);
	}

	void setFallSpeed(const float value) {
		fallSpeed = std::max(value, MIN_SPEED);
	}

	void invert() {
		baseline = 1.f - baseline;
		target = 1.f - target;
		start = current;
		progress = 0.f;
		DEBUG("INVERTED: %d", isInverted());
	}

	bool isInverted() const {
		return baseline < 0.5f;
	}
};


struct Phoenix final : Module {
	enum ParamId {
		HIT_PARAM,
		RISE_PARAM,
		INVERT_PARAM,
		FALL_SPEED_PARAM,
		RISE_SPEED_PARAM,
		HIT_MODE_PARAM,
		FALL_SPEED_CV_PARAM,
		RISE_SPEED_CV_PARAM,
		IO_RANGE_PARAM,
		FALLING_SWITCH_PARAM,
		RISING_SWITCH_PARAM,
		AUX_RANGE_PARAM,
		FALL_LIN_EXP_PARAM,
		RISE_LIN_EXP_PARAM,
		ATT_MODE_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		HIT_INPUT,
		RISE_INPUT,
		INVERT_INPUT,
		MAIN_INPUT,
		FALL_SPEED_CV_INPUT,
		RISE_SPEED_CV_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		MAIN_OUTPUT,
		FALLING_OUTPUT,
		RISING_OUTPUT,
		AUX_OUTPUT,
		FALLEN_OUTPUT,
		RISEN_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(HIT_PARAM_LIGHT, 3),
		ENUMS(RISE_PARAM_LIGHT, 3),
		ENUMS(INVERT_PARAM_LIGHT, 3),
		ENUMS(HIT_MODE_PARAM_LIGHT, 3),
		ENUMS(IO_RANGE_PARAM_LIGHT, 3),
		ENUMS(FALLING_SWITCH_PARAM_LIGHT, 3),
		ENUMS(RISING_SWITCH_PARAM_LIGHT, 3),
		ENUMS(AUX_RANGE_PARAM_LIGHT, 3),
		ENUMS(ATT_MODE_PARAM_LIGHT, 3),
		LIGHTS_LEN
	};

	BaselineTracker baseline;

	dsp::SchmittTrigger hitTrigger, riseTrigger;
	dsp::SchmittTrigger invertTrigger;

	// Buttons
	dsp::BooleanTrigger hitTriggerButton, riseTriggerButton;
	dsp::BooleanTrigger invertTriggerButton;

	Phoenix() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configButton(HIT_PARAM, "Hit");
		configButton(RISE_PARAM , "Rise");
		configParam(INVERT_PARAM, 0.f, 1.f, 0.f, "");
		configParam(FALL_SPEED_PARAM, 0.f, 1.f, 0.f, "");
		configParam(RISE_SPEED_PARAM, 0.f, 1.f, 0.f, "");
		configParam(HIT_MODE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(FALL_SPEED_CV_PARAM, 0.f, 1.f, 0.f, "");
		configParam(RISE_SPEED_CV_PARAM, 0.f, 1.f, 0.f, "");
		configParam(IO_RANGE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(FALLING_SWITCH_PARAM, 0.f, 1.f, 0.f, "");
		configParam(RISING_SWITCH_PARAM, 0.f, 1.f, 0.f, "");
		configParam(AUX_RANGE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(FALL_LIN_EXP_PARAM, 0.f, 1.f, 0.f, "");
		configParam(RISE_LIN_EXP_PARAM, 0.f, 1.f, 0.f, "");
		configParam(ATT_MODE_PARAM, 0.f, 1.f, 0.f, "");
		configInput(HIT_INPUT, "Hit");
		configInput(RISE_INPUT, "Rise");
		configInput(INVERT_INPUT, "");
		configInput(MAIN_INPUT, "Main");
		configInput(FALL_SPEED_CV_INPUT, "");
		configInput(RISE_SPEED_CV_INPUT, "");
		configOutput(MAIN_OUTPUT, "Main");
		configOutput(FALLING_OUTPUT, "");
		configOutput(RISING_OUTPUT, "");
		configOutput(AUX_OUTPUT, "AUX");
		configOutput(FALLEN_OUTPUT, "");
		configOutput(RISEN_OUTPUT, "");
	}

	void process(const ProcessArgs& args) override {
		const bool invertTriggered = invertTriggerButton.process(getParam(INVERT_PARAM).getValue()) ||
			invertTrigger.process(getInput(INVERT_INPUT).getVoltage(), 0.1f, 1.f);

		if (invertTriggered) {
			baseline.invert();
		}

		const bool hitTriggered = hitTriggerButton.process(getParam(HIT_PARAM).getValue()) ||
			hitTrigger.process(getInput(HIT_INPUT).getVoltage(), 0.1f, 1.f);

		if (hitTriggered) {
			baseline.hit(1.f);
		}

		const bool riseTriggered = riseTriggerButton.process(getParam(RISE_PARAM).getValue()) ||
			riseTrigger.process(getInput(RISE_INPUT).getVoltage(), 0.1f, 1.f);

		if (riseTriggered) {
			baseline.recover(1.f);
		}

		baseline.process(args.sampleTime);

		const float input = getInput(MAIN_INPUT).getVoltage();
		const float out = input * baseline.getCurrent();

		getOutput(MAIN_OUTPUT).setVoltage(out);
		getOutput(AUX_OUTPUT).setVoltage(rescale(baseline.getCurrent(), 0.f, 1.f, -5.f, 5.f));

		setLight(HIT_PARAM_LIGHT, hitTriggered ? 1.f : 0.f, args.sampleTime);
		setLight(RISE_PARAM_LIGHT, riseTriggered ? 1.f : 0.f, args.sampleTime);
		setLight(INVERT_PARAM_LIGHT, baseline.isInverted() ? 1.f : 0.f, args.sampleTime);
	}

	void setLight(const LightId id, const float brightness, const float delta) {
		getLight(id + 0).setBrightnessSmooth(brightness, delta);
		getLight(id + 1).setBrightnessSmooth(brightness, delta);
		getLight(id + 2).setBrightnessSmooth(brightness, delta);
	}
};


struct PhoenixWidget final : ModuleWidget {
	explicit PhoenixWidget(Phoenix* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Phoenix.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createLightParamCentered<VCVLightButton<LargeSimpleLight<RedGreenBlueLight>>>(mm2px(Vec(8.25, 25.5)), module, Phoenix::HIT_PARAM, Phoenix::HIT_PARAM_LIGHT));
		addParam(createLightParamCentered<VCVLightButton<LargeSimpleLight<RedGreenBlueLight>>>(mm2px(Vec(32.39, 25.5)), module, Phoenix::RISE_PARAM, Phoenix::RISE_PARAM_LIGHT));
		addParam(createLightParamCentered<VCVLightButton<LargeSimpleLight<RedGreenBlueLight>>>(mm2px(Vec(20.32, 29.574)), module, Phoenix::INVERT_PARAM, Phoenix::INVERT_PARAM_LIGHT));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(8.25, 40.25)), module, Phoenix::FALL_SPEED_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(32.39, 40.25)), module, Phoenix::RISE_SPEED_PARAM));
		addParam(createLightParamCentered<VCVLightButton<MediumSimpleLight<RedGreenBlueLight>>>(mm2px(Vec(20.32, 40.75)), module, Phoenix::HIT_MODE_PARAM, Phoenix::HIT_MODE_PARAM_LIGHT));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(8.25, 50.0)), module, Phoenix::FALL_SPEED_CV_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(32.39, 50.0)), module, Phoenix::RISE_SPEED_CV_PARAM));
		addParam(createLightParamCentered<VCVLightButton<MediumSimpleLight<RedGreenBlueLight>>>(mm2px(Vec(20.32, 65.25)), module, Phoenix::IO_RANGE_PARAM, Phoenix::IO_RANGE_PARAM_LIGHT));
		addParam(createLightParamCentered<VCVLightButton<MediumSimpleLight<RedGreenBlueLight>>>(mm2px(Vec(8.25, 85.0)), module, Phoenix::FALLING_SWITCH_PARAM, Phoenix::FALLING_SWITCH_PARAM_LIGHT));
		addParam(createLightParamCentered<VCVLightButton<MediumSimpleLight<RedGreenBlueLight>>>(mm2px(Vec(32.39, 85.0)), module, Phoenix::RISING_SWITCH_PARAM, Phoenix::RISING_SWITCH_PARAM_LIGHT));
		addParam(createLightParamCentered<VCVLightButton<MediumSimpleLight<RedGreenBlueLight>>>(mm2px(Vec(20.32, 98.711)), module, Phoenix::AUX_RANGE_PARAM, Phoenix::AUX_RANGE_PARAM_LIGHT));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(8.25, 110.25)), module, Phoenix::FALL_LIN_EXP_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(32.39, 110.25)), module, Phoenix::RISE_LIN_EXP_PARAM));
		addParam(createLightParamCentered<VCVLightButton<MediumSimpleLight<RedGreenBlueLight>>>(mm2px(Vec(20.32, 112.75)), module, Phoenix::ATT_MODE_PARAM, Phoenix::ATT_MODE_PARAM_LIGHT));

		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(8.25, 15.574)), module, Phoenix::HIT_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(32.39, 15.574)), module, Phoenix::RISE_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(20.32, 19.824)), module, Phoenix::INVERT_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(20.32, 55.5)), module, Phoenix::MAIN_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(8.25, 59.75)), module, Phoenix::FALL_SPEED_CV_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(32.39, 59.75)), module, Phoenix::RISE_SPEED_CV_INPUT));

		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(20.32, 75.0)), module, Phoenix::MAIN_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(8.25, 75.25)), module, Phoenix::FALLING_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(32.39, 75.25)), module, Phoenix::RISING_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(20.32, 88.961)), module, Phoenix::AUX_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(8.25, 94.75)), module, Phoenix::FALLEN_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(32.39, 94.75)), module, Phoenix::RISEN_OUTPUT));
	}
};


Model* modelPhoenix = createModel<Phoenix, PhoenixWidget>("Phoenix");
