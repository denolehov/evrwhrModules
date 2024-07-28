#include "plugin.hpp"
#include "event.h"

constexpr int LEFT_EXPANDER_SIDE = 0;

enum class ButtonState{ A, B, C };

ButtonState& operator++(ButtonState& state) {
	switch (state) {
		case ButtonState::A: return state = ButtonState::B;
		case ButtonState::B: return state = ButtonState::C;
		case ButtonState::C: return state = ButtonState::A;
	}
	return state;
}

struct Seed : Module {
	enum ParamId {
		FIRST_PARAM,
		SECOND_PARAM,
		THIRD_PARAM,
		FOURTH_PARAM,
		FIFTH_PARAM,
		SIXTH_PARAM,
		RUN_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		CLOCK_INPUT,
		RESET_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(FIRST_LIGHT, 3),
		ENUMS(SECOND_LIGHT, 3),
		ENUMS(THIRD_LIGHT, 3),
		ENUMS(FOURTH_LIGHT, 3),
		ENUMS(FIFTH_LIGHT, 3),
		ENUMS(SIXTH_LIGHT, 3),
		RUN_LIGHT,
		LIGHTS_LEN
	};

	dsp::SchmittTrigger clockTrigger;
	dsp::SchmittTrigger resetTrigger;

	// Seed-related stuff
	int currentSeed;
	const int NUM_SEEDS = 6;
	const ParamId seedParams[6] = {FIRST_PARAM, SECOND_PARAM, THIRD_PARAM, FOURTH_PARAM, FIFTH_PARAM, SIXTH_PARAM};
	const LightId seedLights[6] = {FIRST_LIGHT, SECOND_LIGHT, THIRD_LIGHT, FOURTH_LIGHT, FIFTH_LIGHT, SIXTH_LIGHT};
	ButtonState buttonStates[6] = {ButtonState::A};
	dsp::BooleanTrigger pushTriggers[6];

	Seed() : currentSeed(0) {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configButton(FIRST_PARAM, "A");
		configButton(SECOND_PARAM,  "B");
		configButton(THIRD_PARAM, "C");
		configButton(FOURTH_PARAM,  "D");
		configButton(FIFTH_PARAM, "E");
		configButton(SIXTH_PARAM, "F");
		configSwitch(RUN_PARAM, 0.f, 1.f, 0.f, "Run", {"Run", "Stop"});
		configInput(CLOCK_INPUT, "Clock (24ppqn)");
		configInput(RESET_INPUT, "Reset");
	}

	void onRandomize() override
	{
		for (auto & buttonState : buttonStates)
		{
			const float randomValue = random::uniform();
			if (randomValue < 0.33)
				buttonState = ButtonState::A;
			else if (randomValue < 0.66)
				buttonState = ButtonState::B;
			else
				buttonState = ButtonState::C;
		}
		updateSeed();

		Event ev;
		ev.seed = currentSeed;
		ev.seedChanged = true;
		propagateEvent(ev);
	}

	static bool isExpanderCompatible(const Module* module)
	{
		return module && (module->model == modelRandomWalkLFO || module->model == modelTrigger);
	}

	static bool shouldSendEvent(const Event ev)
	{
		return ev.clock || ev.globalReset || ev.seedChanged;
	}

	void propagateEvent(const Event ev)
	{
		if (!shouldSendEvent(ev))
		{
			return;
		}

		Module* rightModule = getRightExpander().module;
		if (!isExpanderCompatible(rightModule))
			return;

		auto* producerEvent = reinterpret_cast<Event*>(rightModule->getLeftExpander().producerMessage);

		*producerEvent = ev;

		rightModule->getLeftExpander().requestMessageFlip();
	}

	void process(const ProcessArgs& args) override {
		Event ev;

		if (resetTrigger.process(getInput(RESET_INPUT).getVoltage()))
			ev.globalReset = true;

		if (clockTrigger.process(getInput(CLOCK_INPUT).getVoltage()))
		{
			ev.clock = true;
		}

		bool seedChanged = false;
		for (int i = 0; i < NUM_SEEDS; i++) {
			const bool pushed = pushTriggers[i].process(getParam(seedParams[i]).getValue());
			if (pushed) {
				buttonStates[i] = ++buttonStates[i];
				updateSeed();
				seedChanged = true;
			}
			litTheButton(buttonStates[i], seedLights[i], args.sampleTime);
		}
		ev.seed = currentSeed;
		ev.seedChanged = seedChanged;

		propagateEvent(ev);

		// TODO: Incorporate this into the event system
		const float run = getParam(RUN_PARAM).getValue();
		getLight(RUN_LIGHT).setBrightnessSmooth(run, args.sampleTime);
	}

	void updateSeed()
	{
		int seed = 0;
		for (int i = 0; i < NUM_SEEDS; ++i)
		{
			const int stateValue = static_cast<int>(buttonStates[i]) + 1;
			seed ^= stateValue << i * 2;
		}

		currentSeed = seed;
	}

	void litTheButton(ButtonState state, int lightIndex, float delta) {
		switch (state) {
			case ButtonState::A:
				lights[lightIndex + 0].setBrightnessSmooth(0.00, delta);
				lights[lightIndex + 1].setBrightnessSmooth(0.75, delta);
				lights[lightIndex + 2].setBrightnessSmooth(0.00, delta);
				break;
			case ButtonState::B:
				lights[lightIndex + 0].setBrightnessSmooth(0.75, delta);
				lights[lightIndex + 1].setBrightnessSmooth(0.75, delta);
				lights[lightIndex + 2].setBrightnessSmooth(0.00, delta);
				break;
			case ButtonState::C:
				lights[lightIndex + 0].setBrightnessSmooth(0.00, delta);
				lights[lightIndex + 1].setBrightnessSmooth(0.00, delta);
				lights[lightIndex + 2].setBrightnessSmooth(0.75, delta);
				break;
		}
	}

	json_t* dataToJson() override
	{
		json_t* rootJ = json_object();
		const char* seeds[] = {"seed_a", "seed_b", "seed_c", "seed_d", "seed_e", "seed_f", "seed_g", "seed_h"};

		for (int i = 0; i < NUM_SEEDS; ++i)
		{
			json_object_set_new(rootJ, seeds[i], json_integer(static_cast<int>(buttonStates[i])));
		}

		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override
	{
		const char* seeds[] = {"seed_a", "seed_b", "seed_c", "seed_d", "seed_e", "seed_f", "seed_g", "seed_h"};

		for (int i = 0; i < NUM_SEEDS; ++i)
		{
			buttonStates[i] = static_cast<ButtonState>(json_integer_value(json_object_get(rootJ, seeds[i])));
		}
	}};


struct SeedWidget : ModuleWidget {
	SeedWidget(Seed* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Seed.svg")));

		addParam(createLightParamCentered<LightButton<VCVBezel, VCVBezelLight<RedGreenBlueLight>>>(mm2px(Vec(7.62, 15.5)), module, Seed::FIRST_PARAM, Seed::FIRST_LIGHT));
		addParam(createLightParamCentered<LightButton<VCVBezel, VCVBezelLight<RedGreenBlueLight>>>(mm2px(Vec(7.62, 26.5)), module, Seed::SECOND_PARAM, Seed::SECOND_LIGHT));
		addParam(createLightParamCentered<LightButton<VCVBezel, VCVBezelLight<RedGreenBlueLight>>>(mm2px(Vec(7.62, 37.5)), module, Seed::THIRD_PARAM, Seed::THIRD_LIGHT));
		addParam(createLightParamCentered<LightButton<VCVBezel, VCVBezelLight<RedGreenBlueLight>>>(mm2px(Vec(7.62, 48.5)), module, Seed::FOURTH_PARAM, Seed::FOURTH_LIGHT));
		addParam(createLightParamCentered<LightButton<VCVBezel, VCVBezelLight<RedGreenBlueLight>>>(mm2px(Vec(7.62, 59.5)), module, Seed::FIFTH_PARAM, Seed::FIFTH_LIGHT));
		addParam(createLightParamCentered<LightButton<VCVBezel, VCVBezelLight<RedGreenBlueLight>>>(mm2px(Vec(7.62, 70.5)), module, Seed::SIXTH_PARAM, Seed::SIXTH_LIGHT));
		addParam(createLightParamCentered<VCVLightLatch<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(7.62, 81.5)), module, Seed::RUN_PARAM, Seed::RUN_LIGHT));

		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 92.5)), module, Seed::CLOCK_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(7.62, 110.5)), module, Seed::RESET_INPUT));
	}
};


Model* modelSeed = createModel<Seed, SeedWidget>("Seed");