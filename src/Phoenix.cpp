#include "plugin.hpp"

struct BurstDetector {
	dsp::Timer timer;

	float burstiness = 0.f;
	float decayDuration = -std::log(0.01f) / 1.f;
	float sensitivity = .1f;
	float lastTriggerTime = -1.f;

	float process(const float delta, const bool trigger) {
		const float currentTime = timer.process(delta);

		burstiness *= std::exp(-decayDuration * delta);

		if (trigger) {
			if (lastTriggerTime >= 0.f) {
				const float deltaTime = currentTime - lastTriggerTime;
				const float burstIncrement = deltaTime > 0.f ? clamp(1.f / (deltaTime / sensitivity), 0.f, 1.f) : 0;
				burstiness = clamp(burstiness + burstIncrement, 0.f, 1.f);
			}
			lastTriggerTime = currentTime;
		}

		return burstiness;
	}

	void setSensitivity(const float value) {
		this->sensitivity = std::max(value, 1e-6f);
	}

	void setDecayTime(const float seconds) {
		decayDuration = -std::log(0.01f) / seconds;
	}
};

struct Phoenix final : Module {
	enum ParamId {
		SENSITIVITY_PARAM,
		DECAY_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		HIT_INPUT,
		MAIN_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		MAIN_OUTPUT,
		AUX_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	Phoenix() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(SENSITIVITY_PARAM, 1e-6f, 3.f, 0.2f, "Sensitivity", " s");
		configParam(DECAY_PARAM, 0.f, 60.f, 0.f, "Decay", " s");
		configInput(HIT_INPUT, "Hit");
		configInput(MAIN_INPUT, "Main");
		configOutput(MAIN_OUTPUT, "Main");
		configOutput(AUX_OUTPUT, "AUX");
	}

	dsp::SchmittTrigger hitTrigger;

	BurstDetector burstDetector;

	void process(const ProcessArgs& args) override {
		burstDetector.setSensitivity(params[SENSITIVITY_PARAM].getValue());
		burstDetector.setDecayTime(params[DECAY_PARAM].getValue());

		const bool triggered = hitTrigger.process(inputs[HIT_INPUT].getVoltage(), 0.1f, 1.f);
		const float burstiness = burstDetector.process(args.sampleTime, triggered);

		const float in = inputs[MAIN_INPUT].getVoltage();
		outputs[MAIN_OUTPUT].setVoltage(in * burstiness);
		outputs[AUX_OUTPUT].setVoltage(rescale(burstiness, 0.f, 1.f, -5.f, 5.f));
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

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(8.25, 28.101)), module, Phoenix::SENSITIVITY_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(32.39, 28.101)), module, Phoenix::DECAY_PARAM));

		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(8.25, 15.574)), module, Phoenix::HIT_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(20.32, 21.324)), module, Phoenix::MAIN_INPUT));

		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(20.32, 40.824)), module, Phoenix::MAIN_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(20.32, 88.961)), module, Phoenix::AUX_OUTPUT));
	}
};


Model* modelPhoenix = createModel<Phoenix, PhoenixWidget>("Phoenix");
