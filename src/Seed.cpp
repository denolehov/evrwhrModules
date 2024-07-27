#include "plugin.hpp"
#include "OpenSimplexNoise/OpenSimplexNoise.h"

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

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
		SEVENTH_PARAM,
		EIGHTH_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		CLOCK_IN_INPUT,
		RESET_IN_INPUT,
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
		ENUMS(SEVENTH_LIGHT, 3),
		ENUMS(EIGHTH_LIGHT, 3),
		LIGHTS_LEN
	};

	ButtonState buttonStates[8] = {ButtonState::A};

	dsp::BooleanTrigger pushTriggers[8];
	dsp::SchmittTrigger clockTrigger;
	int currentSeed;
	std::unique_ptr<OpenSimplexNoise::Noise> noise;

	Seed() : currentSeed(0), noise(make_unique<OpenSimplexNoise::Noise>(currentSeed)) {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configButton(FIRST_PARAM, "A");
		configButton(SECOND_PARAM,  "B");
		configButton(THIRD_PARAM, "C");
		configButton(FOURTH_PARAM,  "D");
		configButton(FIFTH_PARAM, "E");
		configButton(SIXTH_PARAM, "F");
		configButton(SEVENTH_PARAM, "G");
		configButton(EIGHTH_PARAM,  "H");
		configInput(CLOCK_IN_INPUT, "Clock (24ppqn)");
		configInput(RESET_IN_INPUT, "Reset");
	}

	void onExpanderChange(const ExpanderChangeEvent& e) override
	{
		if (e.side == 0)
			return;

		const Module* rightModule = getRightExpander().module;
		if (!isExpanderCompatible(rightModule))
			return;

		DEBUG("CONNECTED RANDOM LFO!");
	};

	static bool isExpanderCompatible(const Module* module)
	{
		return module && module->model == modelRandomWalkLFO;
	}

	void process(const ProcessArgs& args) override {
		if (clockTrigger.process(getInput(CLOCK_IN_INPUT).getVoltage()))
		{
			double const z = noise->eval(734234.13, 7342.31, 0.0);
			DEBUG("Noise value: %f", rescale(z, -1.0, 1.0, -5.0, 5.0));
		}

		for (int i = 0; i < PARAMS_LEN; i++) {
			const bool pushed = pushTriggers[i].process(getParam(i).getValue());
			if (pushed) {
				buttonStates[i] = ++buttonStates[i];
				reseedNoise();
			}
			litTheButton(buttonStates[i], i * 3, args.sampleTime);
		}
	}

	void reseedNoise()
	{
		updateSeed();
		noise = make_unique<OpenSimplexNoise::Noise>(currentSeed);
		DEBUG("NEW SEED: %d", currentSeed);
	}

	void updateSeed()
	{
		int seed = 0;
		for (int i = 0; i < PARAMS_LEN; ++i)
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
};


struct SeedWidget : ModuleWidget {
	SeedWidget(Seed* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Seed.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createLightParamCentered<LightButton<VCVBezel, VCVBezelLight<RedGreenBlueLight>>>(mm2px(Vec(7.62, 11.5)), module, Seed::FIRST_PARAM, Seed::FIRST_LIGHT));
		addParam(createLightParamCentered<LightButton<VCVBezel, VCVBezelLight<RedGreenBlueLight>>>(mm2px(Vec(7.62, 22.5)), module, Seed::SECOND_PARAM, Seed::SECOND_LIGHT));
		addParam(createLightParamCentered<LightButton<VCVBezel, VCVBezelLight<RedGreenBlueLight>>>(mm2px(Vec(7.62, 33.5)), module, Seed::THIRD_PARAM, Seed::THIRD_LIGHT));
		addParam(createLightParamCentered<LightButton<VCVBezel, VCVBezelLight<RedGreenBlueLight>>>(mm2px(Vec(7.62, 44.5)), module, Seed::FOURTH_PARAM, Seed::FOURTH_LIGHT));
		addParam(createLightParamCentered<LightButton<VCVBezel, VCVBezelLight<RedGreenBlueLight>>>(mm2px(Vec(7.62, 55.5)), module, Seed::FIFTH_PARAM, Seed::FIFTH_LIGHT));
		addParam(createLightParamCentered<LightButton<VCVBezel, VCVBezelLight<RedGreenBlueLight>>>(mm2px(Vec(7.62, 66.5)), module, Seed::SIXTH_PARAM, Seed::SIXTH_LIGHT));
		addParam(createLightParamCentered<LightButton<VCVBezel, VCVBezelLight<RedGreenBlueLight>>>(mm2px(Vec(7.62, 77.5)), module, Seed::SEVENTH_PARAM, Seed::SEVENTH_LIGHT));
		addParam(createLightParamCentered<LightButton<VCVBezel, VCVBezelLight<RedGreenBlueLight>>>(mm2px(Vec(7.62, 88.5)), module, Seed::EIGHTH_PARAM, Seed::EIGHTH_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 101.5)), module, Seed::CLOCK_IN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 114.5)), module, Seed::RESET_IN_INPUT));
	}
};


Model* modelSeed = createModel<Seed, SeedWidget>("Seed");
