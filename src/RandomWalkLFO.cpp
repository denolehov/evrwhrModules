#include "plugin.hpp"
#include "OpenSimplexNoise/OpenSimplexNoise.h"

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

struct RandomWalkLFO : Module {
	enum ParamId {
		LENGTH_PARAM,
		RATE_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		RESET_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		CV_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	RandomWalkLFO() : noise(make_unique<OpenSimplexNoise::Noise>(0)) {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(LENGTH_PARAM, 1.f, 17.f, 1.f, "Length");
		configParam(RATE_PARAM, 0.f, 1.f, 0.5f, "Rate");
		configInput(RESET_INPUT, "Reset");
		configOutput(CV_OUTPUT, "CV");
	}

	double phase = 0.0;
	double realPhase = 0.0;
	std::unique_ptr<OpenSimplexNoise::Noise> noise;

	dsp::SchmittTrigger resetTrigger;
	dsp::SlewLimiter slewLimiter;

	dsp::Timer timer;

	void process(const ProcessArgs& args) override {
		if (resetTrigger.process(getInput(RESET_INPUT).getVoltage())) {
			phase = 0.0f;
		}

		float outCV = noise->eval(1, phase);
		outCV = rescale(outCV, -1.0f, 1.0f, -5.0f, 5.0f);
		getOutput(CV_OUTPUT).setVoltage(outCV);

		const float speedParam = getParam(RATE_PARAM).getValue();
		constexpr float minRate = 0.001f;
		const float maxRate = dsp::FREQ_A4;
		const float speed = minRate * std::pow(maxRate / minRate, speedParam);
		const float phaseIncrement = speed * args.sampleTime;
		phase += phaseIncrement;
	}
};


struct RandomWalkLFOWidget : ModuleWidget {
	RandomWalkLFOWidget(RandomWalkLFO* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/RandomWalkLFO.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackSnapKnob>(mm2px(Vec(7.62, 46.201)), module, RandomWalkLFO::LENGTH_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 64.25)), module, RandomWalkLFO::RATE_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 114.5)), module, RandomWalkLFO::RESET_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 101.5)), module, RandomWalkLFO::CV_OUTPUT));
	}
};


Model* modelRandomWalkLFO = createModel<RandomWalkLFO, RandomWalkLFOWidget>("RandomWalkLFO");