#include <OpenSimplexNoise/OpenSimplexNoise.h>

#include "event.h"
#include "plugin.hpp"

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

enum class LightColor {RED, YELLOW, OFF};

struct Trigger : Module {
	enum ParamId {
		DENSITY_PARAM,
		DIVISION_PARAM,
		VARIANT_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		BLOCK_INPUT,
		RESET_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(DENSITY_LIGHT, 3),
		LIGHTS_LEN
	};

	Event messages[2] = {};
	bool clock = false;

	dsp::PulseGenerator pulse;
	dsp::SchmittTrigger blockTrigger;
	dsp::SchmittTrigger resetTrigger;
	dsp::ClockDivider divider;

	const std::array<uint32_t, 12> divisionMapping = {
		48,	// 1/2
		32,	// 1/2t
		72, // 1/2.
		24, // 1/4
		16, // 1/4t
		36, // 1/4.
		12, // 1/8
		8,	// 1/8t
		18, // 1/8.
		6,	// 1/16
		4,	// 1/16t
		9	// 1/16.
	};

	std::unique_ptr<OpenSimplexNoise::Noise> noise;
	int noisePhase = 0;
	float variant = 0.f;

	Trigger() : noise(make_unique<OpenSimplexNoise::Noise>(0)) {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(DENSITY_PARAM, 0.f, 100.f, 0.f, "Density", "%");
		configSwitch(DIVISION_PARAM, 0.f, 11.f, 0.f, "Division", {"1/2", "1/2t", "1/2.", "1/4", "1/4t", "1/4.", "1/8", "1/8t", "1/8.", "1/16", "1/16t", "1/16."});
		configParam(VARIANT_PARAM, 0.f, 32.f, 0.f, "Variant");
		configInput(BLOCK_INPUT, "Block");
		configInput(RESET_INPUT, "Reset");
		configOutput(OUT_OUTPUT, "Out");

		getLeftExpander().producerMessage = &messages[0];
		getLeftExpander().consumerMessage = &messages[1];
	}

	void onReset(const ResetEvent& e) override
	{
		Module::onReset(e);
		reset();
	}

	void reset()
	{
		pulse.reset();
		divider.reset();
		noisePhase = 0;
	}

	void reseedNoise(int seed) { noise = make_unique<OpenSimplexNoise::Noise>(seed); }

	void handleDivisionChange()
	{
		const auto prevDivision = divider.getDivision();

		const float divisionParam = getParam(DIVISION_PARAM).getValue();
		const int index = static_cast<int>(divisionParam);
		const auto newDivision = divisionMapping[index];

		if (prevDivision != newDivision && noisePhase == 0)
		{
			divider.setDivision(newDivision);
			DEBUG("DIVISION IS SET TO %d", divider.getDivision());
		}
	}

	void handleVariantChange()
	{
		const float newVariant = getParam(VARIANT_PARAM).getValue();
		if (newVariant != variant && noisePhase == 0)
			variant = newVariant;
	}

	void litTheLight(LightId lightIndex, LightColor color, float delta)
	{
		switch (color)
		{
			case LightColor::RED:
				getLight(lightIndex + 0).setBrightnessSmooth(1.f, delta);
				getLight(lightIndex + 1).setBrightnessSmooth(0.f, delta);
				getLight(lightIndex + 2).setBrightnessSmooth(0.f, delta);
			case LightColor::YELLOW:
				getLight(lightIndex + 0).setBrightnessSmooth(1.f, delta);
				getLight(lightIndex + 1).setBrightnessSmooth(1.f, delta);
				getLight(lightIndex + 2).setBrightnessSmooth(0.f, delta);
			case LightColor::OFF:
				getLight(lightIndex + 0).setBrightnessSmooth(0.f, delta);
				getLight(lightIndex + 1).setBrightnessSmooth(0.f, delta);
				getLight(lightIndex + 2).setBrightnessSmooth(0.f, delta);
		}
	}

	void process(const ProcessArgs& args) override {
		const float resetIn = getInput(RESET_INPUT).getVoltage();
		if (resetTrigger.process(resetIn, 0.1f, 2.f))
			reset();

		clock = false;
		handleConsumedMessage();
		handleVariantChange();
		handleDivisionChange();

		if (clock)
			noisePhase++;

		const bool clockTriggered = clock && divider.process();
		const bool isBlocked = getInput(BLOCK_INPUT).getVoltage() >= .1f;

		float noiseVal = noise->eval(variant, noisePhase);
		noiseVal = rescale(noiseVal, -1.0f, 1.0f, 0.0f, 100.0f);

		const float density = getParam(DENSITY_PARAM).getValue();

		bool noiseGate = false;
		if (density >= noiseVal)
			noiseGate = true;

		if (noiseGate && clockTriggered && !isBlocked)
		{
			pulse.trigger(1e-3f);
			litTheLight(DENSITY_LIGHT, LightColor::YELLOW, args.sampleTime);
		} else if (clockTriggered && isBlocked)
		{
			litTheLight(DENSITY_LIGHT, LightColor::RED, args.sampleTime);
		} else
		{
			litTheLight(DENSITY_LIGHT, LightColor::OFF, args.sampleTime);
		}

		getOutput(OUT_OUTPUT).setVoltage(pulse.process(args.sampleTime) ? 10.f : 0.f);
	}

	void handleConsumedMessage()
	{
		auto* ev = reinterpret_cast<Event*>(getLeftExpander().consumerMessage);
		if (!ev || ev->processed)
			return;

		if (ev->seedChanged || ev->globalReset)
		{
			reseedNoise(ev->seed);
			reset();
		}

		if (ev->clock)
			clock = true;

		ev->processed = true;

		propagateToDaisyChained(*ev);
	}

	static bool isExpanderCompatible(const Module* module) // FIXME: This is a duplicate of the same function in Seed.cpp
	{
		return module && (module->model == modelRandomWalkLFO || module->model == modelTrigger);
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

};


struct RoundSmallBlackSnapKnob final : RoundSmallBlackKnob {
	RoundSmallBlackSnapKnob() {
		snap = true;
	}
};

struct RedYellowLight : ModuleLightWidget {
	RedYellowLight() {
		addBaseColor(SCHEME_RED);
		addBaseColor(SCHEME_YELLOW);
	}
};

struct TriggerWidget : ModuleWidget {
	TriggerWidget(Trigger* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Trigger.svg")));

		addParam(createLightParamCentered<VCVLightSlider<RedGreenBlueLight>>(mm2px(Vec(7.62, 22.589)), module, Trigger::DENSITY_PARAM, Trigger::DENSITY_LIGHT));
		// addParam(createLightParamCentered<VCVLightSlider<YellowLight>>(mm2px(Vec(7.62, 22.589)), module, Trigger::DENSITY_PARAM, Trigger::DENSITY_LIGHT));
		addParam(createParamCentered<RoundSmallBlackSnapKnob>(mm2px(Vec(7.62, 46.84)), module, Trigger::DIVISION_PARAM));
		addParam(createParamCentered<RoundSmallBlackSnapKnob>(mm2px(Vec(7.62, 62.755)), module, Trigger::VARIANT_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 78.67)), module, Trigger::BLOCK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 110.5)), module, Trigger::RESET_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 94.585)), module, Trigger::OUT_OUTPUT));
	}
};


Model* modelTrigger = createModel<Trigger, TriggerWidget>("Trigger");
