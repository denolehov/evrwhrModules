#include "plugin.hpp"
#include "OpenSimplexNoise/OpenSimplexNoise.h"
#include "event.h"

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

struct RandomWalkLFO : Module {
	enum ParamId {
		RATE_PARAM,
		VARIANT_PARAM,
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

	dsp::SchmittTrigger resetTrigger;
	dsp::PulseGenerator pulse;
	dsp::Timer timer;
	std::unique_ptr<OpenSimplexNoise::Noise> noise;

	Event messages[2] = {};

	double phase = 0.0;
	float currentVariant = 0.0f;

	RandomWalkLFO() : noise(make_unique<OpenSimplexNoise::Noise>(0)) {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(RATE_PARAM, 0.f, 1.f, 0.5f, "Rate");
		configParam(VARIANT_PARAM, 0.f, 32.f, 0.f, "Variant");
		configInput(RESET_INPUT, "Reset");
		configOutput(CV_OUTPUT, "CV");

		getLeftExpander().producerMessage = &messages[0];
		getLeftExpander().consumerMessage = &messages[1];
	}

	void onReset(const ResetEvent& e) override
	{
		Module::onReset(e);
		doReset();
	}

	void process(const ProcessArgs& args) override {
		handleConsumedMessage();

		const float resetIn = getInput(RESET_INPUT).getVoltage();
		if (resetTrigger.process(resetIn, 0.1f, 2.f))
			doReset();

		handleVariantUpdate();

		float outCV = noise->eval(1, phase + currentVariant);
		outCV = rescale(outCV, -1.0f, 1.0f, -5.0f, 5.0f);
		getOutput(CV_OUTPUT).setVoltage(outCV);

		const float speedParam = getParam(RATE_PARAM).getValue();
		constexpr float minRate = 0.001f;
		const float maxRate = dsp::FREQ_A4;
		const float speed = minRate * std::pow(maxRate / minRate, speedParam);
		const float phaseIncrement = speed * args.sampleTime;
		phase += phaseIncrement;
	}


	static bool isExpanderCompatible(const Module* module) // FIXME: This is a duplicate of the same function in Seed.cpp
	{
		return module && module->model == modelRandomWalkLFO;
	}

	void propagateToDaisyChained(const Event ev)
	{
		Module* rightModule = getRightExpander().module;
		if (!isExpanderCompatible(rightModule))
			return;

		auto* producerEvent = reinterpret_cast<Event*>(rightModule->getLeftExpander().producerMessage);

		*producerEvent = ev;
		producerEvent->processed = false;

		rightModule->getLeftExpander().requestMessageFlip();
	}

	void handleConsumedMessage()
	{
		auto* ev = reinterpret_cast<Event*>(getLeftExpander().consumerMessage);
		if (!ev || ev->processed)
			return;

		if (ev->seedChanged || ev->globalReset)
		{
			reseedNoise(ev->seed);
			doReset();
		}

		ev->processed = true;

		propagateToDaisyChained(*ev);
	}

	void doReset()
	{
		if (phase == 0.0) return;

		phase = 0.0;
	}

	void reseedNoise(int seed) { noise = make_unique<OpenSimplexNoise::Noise>(seed); }

	void handleVariantUpdate()
	{
		const float variant = getParam(VARIANT_PARAM).getValue();
		if (variant == currentVariant)
			return;

		// Variant must be updated once per local or global reset
		// otherwise the output is thrashed while the knob is twisted
		// and that might be *VERY* poor sounding
		if (phase == 0.0f)
			currentVariant = variant;
	}
};


struct RandomWalkLFOWidget : ModuleWidget {
	RandomWalkLFOWidget(RandomWalkLFO* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/RandomWalkLFO.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 19.5)), module, RandomWalkLFO::RATE_PARAM));
		addParam(createParamCentered<RoundBlackSnapKnob>(mm2px(Vec(7.62, 40.5)), module, RandomWalkLFO::VARIANT_PARAM));

		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 108.5)), module, RandomWalkLFO::RESET_INPUT));

		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 89.5)), module, RandomWalkLFO::CV_OUTPUT));
	}
};


Model* modelRandomWalkLFO = createModel<RandomWalkLFO, RandomWalkLFOWidget>("RandomWalkLFO");