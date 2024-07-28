#include "plugin.hpp"
#include "OpenSimplexNoise/OpenSimplexNoise.h"
#include "event.h"

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
		configParam(LENGTH_PARAM, 1.f, 17.f, 17.f, "Length");
		configParam(RATE_PARAM, 0.f, 1.f, 0.5f, "Rate");
		configInput(RESET_INPUT, "Reset");
		configOutput(CV_OUTPUT, "CV");

		getLeftExpander().producerMessage = &messages[0];
		getLeftExpander().consumerMessage = &messages[1];
	}

	double phase = 0.0;
	/**
	 * Represents the number of 24ppqn clock ticks since the last reset.
	 * Resets to 0 when the length is changed or the reset input is triggered.
	 */
	int clock = 0;

	float length = 17.0f;

	std::unique_ptr<OpenSimplexNoise::Noise> noise;

	dsp::SchmittTrigger resetTrigger;

	Event messages[2] = {};

	static bool isExpanderCompatible(const Module* module) // FIXME: This is a duplicate of the same function in Seed.cpp
	{
		return module && module->model == modelRandomWalkLFO;
	}

	void propagateToDaisyChained(const Event ev)
	{
		Module* rightModule = getRightExpander().module;
		if (!isExpanderCompatible(rightModule))
		{
			DEBUG("Right module is not compatible with the LFO expander!");
			return;
		}

		auto* producerEvent = reinterpret_cast<Event*>(rightModule->getLeftExpander().producerMessage);

		*producerEvent = ev;
		producerEvent->processed = false;

		rightModule->getLeftExpander().requestMessageFlip();

		DEBUG("Sent a message to right expander from LFO!");
	}

	void handleConsumedMessage()
	{
		auto* ev = reinterpret_cast<Event*>(getLeftExpander().consumerMessage);
		if (!ev || ev->processed)
			return;

		DEBUG("Received message from left expander: seed: %d, clock: %d", ev->seed, ev->clock);

		if (ev->seedChanged || ev->globalReset)
		{
			reseedNoise(ev->seed);
			reset();
		}

		if (ev->clock)
			clock++;

		ev->processed = true;
		propagateToDaisyChained(*ev);
	}

	void reset()
	{
		phase = 0.0f;
		clock = 0;
	}

	void reseedNoise(int seed)
	{
		noise = make_unique<OpenSimplexNoise::Noise>(seed);
	}

	void process(const ProcessArgs& args) override {
		handleConsumedMessage();

		if (resetTrigger.process(getInput(RESET_INPUT).getVoltage())) {
			reset();
		}

		const float newLength = getParam(LENGTH_PARAM).getValue();
		if (length != newLength)
		{
			DEBUG("Length changed from %f to %f", length, newLength);
			length = newLength;
		}

		// the length is in bars, meaning that when the clock reached 96 * length, it should reset
		if (clock >= 96 * length)
			reset();

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