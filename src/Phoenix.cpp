#include "plugin.hpp"

struct BaselineTracker {
	enum State { IDLE, FALLING, RISING } state = IDLE;

	float baseline = 1.f;
	float start = 1.f, current = 1.f, target = 1.f;
	float progress = 0.f;

	float MIN_SPEED = 0.0f;
	float fallSpeed = MIN_SPEED, riseSpeed = MIN_SPEED;

	float riseLinExpRatio = 0.f, fallLinExpRatio = 0.f;

	void updateState() {
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

	struct Result {
		bool hasRisen = false, hasFallen = false;
		float current = 0.f;

		Result() = default;
		Result(bool hasRisen, bool hasFallen, float current) : hasRisen(hasRisen), hasFallen(hasFallen), current(current) {}
	};

	Result process(const float delta) {
		updateState();

		if (state == IDLE) {
			return {false, false, current};
		}

		progress += delta;
		const float speed = state == RISING ? riseSpeed : fallSpeed;
		float p = 1.f;
		if (speed > 0.f) {
			p = clamp(progress / speed, 0.f, 1.f);
		}

		p = crossfade(p, easeInAndOut(p), state == RISING ? riseLinExpRatio : fallLinExpRatio);

		current = crossfade(start, target, p);

		if (fabs(target - baseline) < 0.0001f) {
			target = baseline;
		}

		if (fabs(current - target) < 0.0001f) {
			current = target;
		}

		const bool hasRisen = current == baseline;
		const bool hasFallen = current == getInvertedBaseline();

		return { hasRisen, hasFallen, current };
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
		baseline = getInvertedBaseline();
		target = 1.f - target;
		start = current;
		progress = 0.f;
	}

	bool isInverted() const {
		return baseline < 0.5f;
	}

	float getInvertedBaseline() const {
		return 1.f - baseline;
	}

	void setRiseLinExpRatio(const float value) {
		riseLinExpRatio = value;
	}

	void setFallLinExpRatio(const float value) {
		fallLinExpRatio = value;
	}

	static float easeInAndOut(const float t) {
		if (t < 0.5f) {
			return 4 * t * t * t;
		}

		return (t - 1) * (2 * t - 2) * (2 * t - 2) + 1;
	}
};


struct Phoenix final : Module {
	enum ParamId {
		FALL_PARAM,
		RISE_PARAM,
		IO_RANGE_PARAM,
		HIT_CV_PARAM,
		RISE_CV_PARAM,
		HIT_MODE_PARAM,
		FALL_SPEED_PARAM,
		RISE_SPEED_PARAM,
		FALL_SPEED_CV_PARAM,
		RISE_SPEED_CV_PARAM,
		INVERT_PARAM,
		AUX_RANGE_PARAM,
		FALL_LIN_EXP_PARAM,
		RISE_LIN_EXP_PARAM,
		ATT_MODE_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		HIT_INPUT,
		RISE_INPUT,
		MAIN_INPUT,
		HIT_CV_INPUT,
		RISE_CV_INPUT,
		INVERT_INPUT,
		FALL_SPEED_CV_INPUT,
		RISE_SPEED_CV_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		MAIN_OUTPUT,
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

	struct Range {
		float min, max;
	};

	enum RangeMode { BI_10, BI_5, UNI_10, UNI_5, NUM_RANGE_MODES };

	std::map<RangeMode, Range> ioRanges = {
		{BI_10, {-10.f, 10.f}},
		{BI_5, {-5.f, 5.f}},
		{UNI_10, {0.f, 10.f}},
		{UNI_5, {0.f, 5.f}}
	};

	RangeMode ioRangeMode = BI_10;
	RangeMode auxRangeMode = BI_5;
	Range ioRange = ioRanges[ioRangeMode];
	Range auxRange = ioRanges[auxRangeMode];

	enum AttenuationMode { VCA, Nudge, NUM_ATT_MODES } attMode = VCA;

	BaselineTracker baseline;

	// Input triggers
	dsp::SchmittTrigger hitTrigger, riseTrigger;
	dsp::SchmittTrigger invertTrigger;

	// Output triggers
	dsp::PulseGenerator fallenTrigger, risenTrigger;

	// Buttons
	dsp::BooleanTrigger invertTriggerButton;
	dsp::BooleanTrigger ioRangeButton, auxRangeButton;
	dsp::BooleanTrigger attModeButton;

	// Min, Max, Defaults
	const float MIN_SPEED = 1e-3f, MAX_SPEED = 60.f, DEFAULT_SPEED = 1.f;
	const float MIN_STRENGTH = 1e-3f, MAX_STRENGTH = 1.f, DEFAULT_STRENGTH = 1.f;

	Phoenix() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		configInput(HIT_INPUT, "Hit");
		configInput(RISE_INPUT, "Rise");
		configInput(HIT_CV_INPUT, "Hit strength CV");
		configInput(RISE_CV_INPUT, "Rise strength CV");

		configParam(FALL_PARAM, MIN_STRENGTH, MAX_STRENGTH, DEFAULT_STRENGTH, "Hit strength");
		configParam(HIT_CV_PARAM, -1.f, 1.f, 0.f, "Hit strength CV");

		configParam(RISE_PARAM, MIN_STRENGTH, MAX_STRENGTH, DEFAULT_STRENGTH, "Rise strength");
		configParam(RISE_CV_PARAM, -1.f, 1.f, 0.f, "Hit strength CV");

		configParam(FALL_SPEED_PARAM, std::log2(MIN_SPEED), std::log2(MAX_SPEED), std::log2(DEFAULT_SPEED), "Fall speed", " s", 2);
		configParam(FALL_SPEED_CV_PARAM, -1.f, 1.f, 0.f, "Fall speed CV");
		configInput(FALL_SPEED_CV_INPUT, "Fall speed CV");
		configParam(FALL_LIN_EXP_PARAM, 0.f, 1.f, 0.f, "Linear / Exponential fall");

		configParam(RISE_SPEED_PARAM, std::log2(MIN_SPEED), std::log2(MAX_SPEED), std::log2(DEFAULT_SPEED), "Rise speed", " s", 2);
		configParam(RISE_SPEED_CV_PARAM, -1.f, 1.f, 0.f, "Rise speed CV");
		configInput(RISE_SPEED_CV_INPUT, "Rise speed CV");
		configParam(RISE_LIN_EXP_PARAM, 0.f, 1.f, 0.f, "Linear / Exponential rise");

		configButton(INVERT_PARAM, "Invert");
		configInput(INVERT_INPUT, "Invert");

		configButton(IO_RANGE_PARAM, "Input / Output range");
		configButton(HIT_MODE_PARAM, "Hit mode");
		configButton(AUX_RANGE_PARAM, "AUX range");
		configButton(ATT_MODE_PARAM, "Attenuation mode");

		configInput(MAIN_INPUT, "Main");
		configOutput(MAIN_OUTPUT, "Main");
		configOutput(AUX_OUTPUT, "AUX");
		configOutput(FALLEN_OUTPUT, "Fallen");
		configOutput(RISEN_OUTPUT, "Risen");
	}

	void process(const ProcessArgs& args) override {
		const float fallSpeed = std::exp2(getParam(FALL_SPEED_PARAM).getValue());
		baseline.setFallSpeed(attenuvert(fallSpeed, FALL_SPEED_CV_INPUT, FALL_SPEED_CV_PARAM, MIN_SPEED, MAX_SPEED));

		const float riseSpeed = std::exp2(getParam(RISE_SPEED_PARAM).getValue());
		baseline.setRiseSpeed(attenuvert(riseSpeed, RISE_SPEED_CV_INPUT, RISE_SPEED_CV_PARAM, MIN_SPEED, MAX_SPEED));

		baseline.setFallLinExpRatio(getParam(FALL_LIN_EXP_PARAM).getValue());
		baseline.setRiseLinExpRatio(getParam(RISE_LIN_EXP_PARAM).getValue());

		const bool invertTriggered = invertTriggerButton.process(getParam(INVERT_PARAM).getValue()) ||
			invertTrigger.process(getInput(INVERT_INPUT).getVoltage(), 0.1f, 1.f);
		if (invertTriggered) {
			baseline.invert();
		}

		const bool hitTriggered = hitTrigger.process(getInput(HIT_INPUT).getVoltage(), 0.1f, 1.f);
		if (hitTriggered) {
			const float hitStrength = attenuvert(getParam(FALL_PARAM).getValue(), HIT_CV_INPUT, HIT_CV_PARAM, MIN_STRENGTH, MAX_STRENGTH);
			baseline.hit(hitStrength);
		}

		const bool riseTriggered = riseTrigger.process(getInput(RISE_INPUT).getVoltage(), 0.1f, 1.f);
		if (riseTriggered) {
			const float riseStrength = attenuvert(getParam(RISE_PARAM).getValue(), RISE_CV_INPUT, RISE_CV_PARAM, MIN_STRENGTH, MAX_STRENGTH);
			baseline.recover(riseStrength);
		}

		const bool ioRangeChanged = ioRangeButton.process(getParam(IO_RANGE_PARAM).getValue());
		if (ioRangeChanged) {
			ioRangeMode = static_cast<RangeMode>((ioRangeMode + 1) % NUM_RANGE_MODES);
			ioRange = ioRanges[ioRangeMode];
		}

		const bool auxRangeChanged = auxRangeButton.process(getParam(AUX_RANGE_PARAM).getValue());
		if (auxRangeChanged) {
			auxRangeMode = static_cast<RangeMode>((auxRangeMode + 1) % NUM_RANGE_MODES);
			auxRange = ioRanges[auxRangeMode];
		}

		const bool attModeChanged = attModeButton.process(getParam(ATT_MODE_PARAM).getValue());
		if (attModeChanged) {
			attMode = static_cast<AttenuationMode>((attMode + 1) % NUM_ATT_MODES);
		}

		const BaselineTracker::Result result = baseline.process(args.sampleTime);

		const float input = clamp(getInput(MAIN_INPUT).getVoltage(), ioRange.min, ioRange.max);

		float out = input;
		switch (attMode) {
			case VCA:
				out *= baseline.getCurrent();
				break;
			case Nudge:
			default:
				const float new_max = rescale(baseline.getCurrent(), 0.f, 1.f, ioRange.min, ioRange.max);
				out = rescale(input, ioRange.min, ioRange.max, ioRange.min, new_max);
				break;
		}

		getOutput(MAIN_OUTPUT).setVoltage(clamp(out, ioRange.min, ioRange.max));
		getOutput(AUX_OUTPUT).setVoltage(rescale(baseline.getCurrent(), 0.f, 1.f, auxRange.min, auxRange.max));

		if (result.hasFallen) fallenTrigger.trigger();
		getOutput(FALLEN_OUTPUT).setVoltage(fallenTrigger.process(args.sampleTime) ? 10.f : 0.f);

		if (result.hasRisen) risenTrigger.trigger();  // HE HAS RISEN
		getOutput(RISEN_OUTPUT).setVoltage(risenTrigger.process(args.sampleTime) ? 10.f : 0.f);

		setLight(HIT_PARAM_LIGHT, hitTriggered ? 1.f : 0.f, WHITE, args.sampleTime);
		setLight(RISE_PARAM_LIGHT, riseTriggered ? 1.f : 0.f, WHITE, args.sampleTime);
		setLight(INVERT_PARAM_LIGHT, baseline.isInverted() ? 1.f : 0.f, WHITE, args.sampleTime);

		setLight(IO_RANGE_PARAM_LIGHT, 1.f, rangeColors[ioRangeMode], args.sampleTime);
		setLight(AUX_RANGE_PARAM_LIGHT, 1.f, rangeColors[auxRangeMode], args.sampleTime);

		setLight(ATT_MODE_PARAM_LIGHT, 1.f, attColors[attMode], args.sampleTime);
	}

	float attenuvert(const float param, const InputId cvInput, const ParamId cvParam, const float paramMin, const float paramMax) {
		if (!getInput(cvInput).isConnected()) {
			return param;
		}

		const float att = getParam(cvParam).getValue();
		float in = getInput(cvInput).getVoltage();
		in = rescale(in, -5.f, 5.f, paramMin, paramMax);

		return clamp(param + in * att, paramMin, paramMax);
	}

	enum Color { WHITE, DARK_BLUE, TEAL, BRIGHT_ORANGE, LIME_GREEN, BRIGHT_YELLOW, DEEP_RED };
	struct RGB { float r, g, b; };

	std::map<Color, RGB> colors = {
		{WHITE, {1.f, 1.f, 1.f}},
		{DARK_BLUE, {0.0f, 0.2f, 0.6f}},
		{TEAL, {0.0f, 0.502f, 0.502f}},
		{BRIGHT_ORANGE, {1.f, 0.647f, 0.0f}},
		{LIME_GREEN, {0.196f, 0.804f, 0.196f}},
		{BRIGHT_YELLOW, {1.f, 0.85f, 0.0f}},
		{DEEP_RED, {0.8f, 0.0f, 0.0f}},
	};

	std::map<RangeMode, Color> rangeColors = {
		{BI_10, DARK_BLUE},
		{BI_5, TEAL},
		{UNI_10, BRIGHT_ORANGE},
		{UNI_5, LIME_GREEN}
	};

	std::map<AttenuationMode, Color> attColors = {
		{VCA, BRIGHT_YELLOW},
		{Nudge, DEEP_RED}
	};

	void setLight(const LightId id, const float brightness, Color color, const float delta) {
		const RGB& rgb = colors[color];

		getLight(id + 0).setBrightnessSmooth(brightness * rgb.r, delta);
		getLight(id + 1).setBrightnessSmooth(brightness * rgb.g, delta);
		getLight(id + 2).setBrightnessSmooth(brightness * rgb.b, delta);
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

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(8.25, 28.101)), module, Phoenix::FALL_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(32.39, 28.101)), module, Phoenix::RISE_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(8.25, 60.923)), module, Phoenix::FALL_SPEED_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(32.39, 60.923)), module, Phoenix::RISE_SPEED_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(8.25, 110.25)), module, Phoenix::FALL_LIN_EXP_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(32.39, 110.25)), module, Phoenix::RISE_LIN_EXP_PARAM));

		addParam(createLightParamCentered<VCVLightButton<LargeSimpleLight<RedGreenBlueLight>>>(mm2px(Vec(20.32, 75.5)), module, Phoenix::INVERT_PARAM, Phoenix::INVERT_PARAM_LIGHT));
		addParam(createLightParamCentered<VCVLightButton<LargeSimpleLight<RedGreenBlueLight>>>(mm2px(Vec(20.32, 53.137)), module, Phoenix::HIT_MODE_PARAM, Phoenix::HIT_MODE_PARAM_LIGHT));
		addParam(createLightParamCentered<VCVLightButton<LargeSimpleLight<RedGreenBlueLight>>>(mm2px(Vec(20.32, 31.074)), module, Phoenix::IO_RANGE_PARAM, Phoenix::IO_RANGE_PARAM_LIGHT));
		addParam(createLightParamCentered<VCVLightButton<LargeSimpleLight<RedGreenBlueLight>>>(mm2px(Vec(20.32, 98.711)), module, Phoenix::AUX_RANGE_PARAM, Phoenix::AUX_RANGE_PARAM_LIGHT));
		addParam(createLightParamCentered<VCVLightButton<LargeSimpleLight<RedGreenBlueLight>>>(mm2px(Vec(20.32, 112.75)), module, Phoenix::ATT_MODE_PARAM, Phoenix::ATT_MODE_PARAM_LIGHT));

		addParam(createParamCentered<Trimpot>(mm2px(Vec(8.25, 37.851)), module, Phoenix::HIT_CV_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(32.39, 37.851)), module, Phoenix::RISE_CV_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(8.25, 70.673)), module, Phoenix::FALL_SPEED_CV_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(32.39, 70.673)), module, Phoenix::RISE_SPEED_CV_PARAM));

		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(8.25, 15.574)), module, Phoenix::HIT_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(32.39, 15.574)), module, Phoenix::RISE_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(20.32, 21.324)), module, Phoenix::MAIN_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(8.25, 47.777)), module, Phoenix::HIT_CV_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(32.39, 47.777)), module, Phoenix::RISE_CV_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(20.32, 65.75)), module, Phoenix::INVERT_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(8.25, 80.423)), module, Phoenix::FALL_SPEED_CV_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(32.39, 80.423)), module, Phoenix::RISE_SPEED_CV_INPUT));

		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(20.32, 40.824)), module, Phoenix::MAIN_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(20.32, 88.961)), module, Phoenix::AUX_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(8.25, 94.75)), module, Phoenix::FALLEN_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(32.39, 94.75)), module, Phoenix::RISEN_OUTPUT));
	}
};


Model* modelPhoenix = createModel<Phoenix, PhoenixWidget>("Phoenix");
