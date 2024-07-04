#include "plugin.hpp"


struct StereoMatrixMixer : Module {
	enum ParamId {
		ATT11_PARAM,
		ATT21_PARAM,
		ATT31_PARAM,
		ATT41_PARAM,
		MIX11_PARAM,
		MIX21_PARAM,
		MIX31_PARAM,
		MIX41_PARAM,
		ATT12_PARAM,
		ATT22_PARAM,
		ATT32_PARAM,
		ATT42_PARAM,
		MIX12_PARAM,
		MIX22_PARAM,
		MIX32_PARAM,
		MIX42_PARAM,
		ATT13_PARAM,
		ATT23_PARAM,
		ATT33_PARAM,
		ATT43_PARAM,
		MIX13_PARAM,
		MIX23_PARAM,
		MIX33_PARAM,
		MIX43_PARAM,
		ATT14_PARAM,
		ATT24_PARAM,
		ATT34_PARAM,
		ATT44_PARAM,
		MIX14_PARAM,
		MIX24_PARAM,
		MIX34_PARAM,
		MIX44_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		L1_INPUT,
		R1_INPUT,
		MOD11_INPUT,
		MOD21_INPUT,
		MOD31_INPUT,
		MOD41_INPUT,
		L2_INPUT,
		R2_INPUT,
		MOD12_INPUT,
		MOD22_INPUT,
		MOD32_INPUT,
		MOD42_INPUT,
		L3_INPUT,
		R3_INPUT,
		MOD13_INPUT,
		MOD23_INPUT,
		MOD33_INPUT,
		MOD43_INPUT,
		L4_INPUT,
		R4_INPUT,
		MOD14_INPUT,
		MOD24_INPUT,
		MOD34_INPUT,
		MOD44_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OL1_OUTPUT,
		OR1_OUTPUT,
		OL2_OUTPUT,
		OR2_OUTPUT,
		OL3_OUTPUT,
		OR3_OUTPUT,
		OL4_OUTPUT,
		OR4_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(LED11_LIGHT, 3),
		ENUMS(LED21_LIGHT, 3),
		ENUMS(LED31_LIGHT, 3),
		ENUMS(LED41_LIGHT, 3),
		ENUMS(LED12_LIGHT, 3),
		ENUMS(LED22_LIGHT, 3),
		ENUMS(LED32_LIGHT, 3),
		ENUMS(LED42_LIGHT, 3),
		ENUMS(LED13_LIGHT, 3),
		ENUMS(LED23_LIGHT, 3),
		ENUMS(LED33_LIGHT, 3),
		ENUMS(LED43_LIGHT, 3),
		ENUMS(LED14_LIGHT, 3),
		ENUMS(LED24_LIGHT, 3),
		ENUMS(LED34_LIGHT, 3),
		ENUMS(LED44_LIGHT, 3),
		LIGHTS_LEN
	};

	StereoMatrixMixer() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(ATT11_PARAM, -1.0f, 1.f, 0.f, "");
		configParam(ATT21_PARAM, -1.0f, 1.f, 0.f, "");
		configParam(ATT31_PARAM, -1.0f, 1.f, 0.f, "");
		configParam(ATT41_PARAM, -1.0f, 1.f, 0.f, "");
		configParam(MIX11_PARAM, -1.0f, 1.f, 0.f, "");
		configParam(MIX21_PARAM, -1.0f, 1.f, 0.f, "");
		configParam(MIX31_PARAM, -1.0f, 1.f, 0.f, "");
		configParam(MIX41_PARAM, -1.0f, 1.f, 0.f, "");
		configParam(ATT12_PARAM, -1.0f, 1.f, 0.f, "");
		configParam(ATT22_PARAM, -1.0f, 1.f, 0.f, "");
		configParam(ATT32_PARAM, -1.0f, 1.f, 0.f, "");
		configParam(ATT42_PARAM, -1.0f, 1.f, 0.f, "");
		configParam(MIX12_PARAM, -1.0f, 1.f, 0.f, "");
		configParam(MIX22_PARAM, -1.0f, 1.f, 0.f, "");
		configParam(MIX32_PARAM, -1.0f, 1.f, 0.f, "");
		configParam(MIX42_PARAM, -1.0f, 1.f, 0.f, "");
		configParam(ATT13_PARAM, -1.0f, 1.f, 0.f, "");
		configParam(ATT23_PARAM, -1.0f, 1.f, 0.f, "");
		configParam(ATT33_PARAM, -1.0f, 1.f, 0.f, "");
		configParam(ATT43_PARAM, -1.0f, 1.f, 0.f, "");
		configParam(MIX13_PARAM, -1.0f, 1.f, 0.f, "");
		configParam(MIX23_PARAM, -1.0f, 1.f, 0.f, "");
		configParam(MIX33_PARAM, -1.0f, 1.f, 0.f, "");
		configParam(MIX43_PARAM, -1.0f, 1.f, 0.f, "");
		configParam(ATT14_PARAM, -1.0f, 1.f, 0.f, "");
		configParam(ATT24_PARAM, -1.0f, 1.f, 0.f, "");
		configParam(ATT34_PARAM, -1.0f, 1.f, 0.f, "");
		configParam(ATT44_PARAM, -1.0f, 1.f, 0.f, "");
		configParam(MIX14_PARAM, -1.0f, 1.f, 0.f, "");
		configParam(MIX24_PARAM, -1.0f, 1.f, 0.f, "");
		configParam(MIX34_PARAM, -1.0f, 1.f, 0.f, "");
		configParam(MIX44_PARAM, -1.0f, 1.f, 0.f, "");
		configInput(L1_INPUT, "");
		configInput(R1_INPUT, "");
		configInput(MOD11_INPUT, "");
		configInput(MOD21_INPUT, "");
		configInput(MOD31_INPUT, "");
		configInput(MOD41_INPUT, "");
		configInput(L2_INPUT, "");
		configInput(R2_INPUT, "");
		configInput(MOD12_INPUT, "");
		configInput(MOD22_INPUT, "");
		configInput(MOD32_INPUT, "");
		configInput(MOD42_INPUT, "");
		configInput(L3_INPUT, "");
		configInput(R3_INPUT, "");
		configInput(MOD13_INPUT, "");
		configInput(MOD23_INPUT, "");
		configInput(MOD33_INPUT, "");
		configInput(MOD43_INPUT, "");
		configInput(L4_INPUT, "");
		configInput(R4_INPUT, "");
		configInput(MOD14_INPUT, "");
		configInput(MOD24_INPUT, "");
		configInput(MOD34_INPUT, "");
		configInput(MOD44_INPUT, "");
		configOutput(OL1_OUTPUT, "");
		configOutput(OR1_OUTPUT, "");
		configOutput(OL2_OUTPUT, "");
		configOutput(OR2_OUTPUT, "");
		configOutput(OL3_OUTPUT, "");
		configOutput(OR3_OUTPUT, "");
		configOutput(OL4_OUTPUT, "");
		configOutput(OR4_OUTPUT, "");
	}

	static constexpr float mixParams[4][4] = {
		{MIX11_PARAM, MIX21_PARAM, MIX31_PARAM, MIX41_PARAM},
		{MIX12_PARAM, MIX22_PARAM, MIX32_PARAM, MIX42_PARAM},
		{MIX13_PARAM, MIX23_PARAM, MIX33_PARAM, MIX43_PARAM},
		{MIX14_PARAM, MIX24_PARAM, MIX34_PARAM, MIX44_PARAM},
	};

	static constexpr float attParams[4][4] = {
		{ATT11_PARAM, ATT21_PARAM, ATT31_PARAM, ATT41_PARAM},
		{ATT12_PARAM, ATT22_PARAM, ATT32_PARAM, ATT42_PARAM},
		{ATT13_PARAM, ATT23_PARAM, ATT33_PARAM, ATT43_PARAM},
		{ATT14_PARAM, ATT24_PARAM, ATT34_PARAM, ATT44_PARAM},
	};

	static constexpr float modInputs[4][4] = {
		{MOD11_INPUT, MOD21_INPUT, MOD31_INPUT, MOD41_INPUT},
		{MOD12_INPUT, MOD22_INPUT, MOD32_INPUT, MOD42_INPUT},
		{MOD13_INPUT, MOD23_INPUT, MOD33_INPUT, MOD43_INPUT},
		{MOD14_INPUT, MOD24_INPUT, MOD34_INPUT, MOD44_INPUT},
	};

	static constexpr float mixLights[4][4] = {
		{LED11_LIGHT, LED21_LIGHT, LED31_LIGHT, LED41_LIGHT},
		{LED12_LIGHT, LED22_LIGHT, LED32_LIGHT, LED42_LIGHT},
		{LED13_LIGHT, LED23_LIGHT, LED33_LIGHT, LED43_LIGHT},
		{LED14_LIGHT, LED24_LIGHT, LED34_LIGHT, LED44_LIGHT},
	};

	void process(const ProcessArgs& args) override {
		float outL[4] = {0.f, 0.f, 0.f, 0.f};
		float outR[4] = {0.f, 0.f, 0.f, 0.f};

		const float inL[4] = {
			inputs[L1_INPUT].getVoltage(),
			inputs[L2_INPUT].getVoltage(),
			inputs[L3_INPUT].getVoltage(),
			inputs[L4_INPUT].getVoltage(),
		};

		const float inR[4] = {
			inputs[R1_INPUT].isConnected() ? inputs[R1_INPUT].getVoltage() : inL[0],
			inputs[R2_INPUT].isConnected() ? inputs[R2_INPUT].getVoltage() : inL[1],
			inputs[R3_INPUT].isConnected() ? inputs[R3_INPUT].getVoltage() : inL[2],
			inputs[R4_INPUT].isConnected() ? inputs[R4_INPUT].getVoltage() : inL[3],
		};

		for (int i = 0; i < 4; i++)
		{
			float mixL = 0.f;
			float mixR = 0.f;

			float cumulativeMixL[4] = {0.f, 0.f, 0.f, 0.f};
			float cumulativeMixR[4] = {0.f, 0.f, 0.f, 0.f};

			for (int j = 0; j < 4; j++)
			{
				const float mixFactor = getMixFactor(j, i);

				mixL += mixFactor * inL[j];
				mixR += mixFactor * inR[j];

				cumulativeMixL[j] += mixL;
				cumulativeMixR[j] += mixR;

				float mixAvg = (cumulativeMixL[j] + cumulativeMixR[j]) / 2.f;

				// In real life, the lights are dim, so offset it a bit so they're more visible.
				float brightness = fabs(mixAvg) / 10.f + 0.15f;
				brightness = clamp(brightness, 0.f, 1.f);

				if (mixAvg > 0.f)
				{
					lights[mixLights[j][i] + 1].setBrightness(brightness);
				} else if (mixAvg < 0.f)
				{
					lights[mixLights[j][i] + 0].setBrightness(brightness);
				} else
				{
					lights[mixLights[j][i]].setBrightness(0.f);
					lights[mixLights[j][i] + 1].setBrightness(0.f);
				}
			}

			outL[i] = clamp(mixL, -10.f, 10.f);
			outR[i] = clamp(mixR, -10.f, 10.f);
		}

		outputs[OL1_OUTPUT].setVoltage(outL[0]);
		outputs[OR1_OUTPUT].setVoltage(outR[0]);
		outputs[OL2_OUTPUT].setVoltage(outL[1]);
		outputs[OR2_OUTPUT].setVoltage(outR[1]);
		outputs[OL3_OUTPUT].setVoltage(outL[2]);
		outputs[OR3_OUTPUT].setVoltage(outR[2]);
		outputs[OL4_OUTPUT].setVoltage(outL[3]);
		outputs[OR4_OUTPUT].setVoltage(outR[3]);
	}

	float getMixFactor(const int row, const int col)
	{
		const float mixFactor = params[mixParams[row][col]].getValue();
		const float modValue = inputs[modInputs[row][col]].isConnected() ? inputs[modInputs[row][col]].getVoltage() / 5.f : 0.f;
		const float attFactor = params[attParams[row][col]].getValue();

		return mixFactor + attFactor * modValue;
	}
};


struct StereoMatrixMixerWidget : ModuleWidget {
	StereoMatrixMixerWidget(StereoMatrixMixer* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/StereoMatrixMixer.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(28.0, 20.0)), module, StereoMatrixMixer::MIX11_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(62.0, 20.0)), module, StereoMatrixMixer::MIX21_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(96.0, 20.0)), module, StereoMatrixMixer::MIX31_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(130.0, 20.0)), module, StereoMatrixMixer::MIX41_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(28.0, 46.0)), module, StereoMatrixMixer::MIX12_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(62.0, 46.0)), module, StereoMatrixMixer::MIX22_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(96.0, 46.0)), module, StereoMatrixMixer::MIX32_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(130.0, 46.0)), module, StereoMatrixMixer::MIX42_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(28.0, 72.0)), module, StereoMatrixMixer::MIX13_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(62.0, 72.0)), module, StereoMatrixMixer::MIX23_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(96.0, 72.0)), module, StereoMatrixMixer::MIX33_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(130.0, 72.0)), module, StereoMatrixMixer::MIX43_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(28.0, 98.0)), module, StereoMatrixMixer::MIX14_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(62.0, 98.0)), module, StereoMatrixMixer::MIX24_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(96.0, 98.0)), module, StereoMatrixMixer::MIX34_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(130.0, 98.0)), module, StereoMatrixMixer::MIX44_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(44.0, 14.0)), module, StereoMatrixMixer::ATT11_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(78.0, 14.0)), module, StereoMatrixMixer::ATT21_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(112.0, 14.0)), module, StereoMatrixMixer::ATT31_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(146.0, 14.0)), module, StereoMatrixMixer::ATT41_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(44.0, 40.0)), module, StereoMatrixMixer::ATT12_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(78.0, 40.0)), module, StereoMatrixMixer::ATT22_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(112.0, 40.0)), module, StereoMatrixMixer::ATT32_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(146.0, 40.0)), module, StereoMatrixMixer::ATT42_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(44.0, 66.0)), module, StereoMatrixMixer::ATT13_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(78.0, 66.0)), module, StereoMatrixMixer::ATT23_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(112.0, 66.0)), module, StereoMatrixMixer::ATT33_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(146.0, 66.0)), module, StereoMatrixMixer::ATT43_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(44.0, 92.0)), module, StereoMatrixMixer::ATT14_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(78.0, 92.0)), module, StereoMatrixMixer::ATT24_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(112.0, 92.0)), module, StereoMatrixMixer::ATT34_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(146.0, 92.0)), module, StereoMatrixMixer::ATT44_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.0, 14.0)), module, StereoMatrixMixer::L1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.0, 26.0)), module, StereoMatrixMixer::R1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(44.0, 26.0)), module, StereoMatrixMixer::MOD11_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(78.0, 26.0)), module, StereoMatrixMixer::MOD21_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(112.0, 26.0)), module, StereoMatrixMixer::MOD31_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(146.0, 26.0)), module, StereoMatrixMixer::MOD41_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.0, 40.0)), module, StereoMatrixMixer::L2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.0, 52.0)), module, StereoMatrixMixer::R2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(44.0, 52.0)), module, StereoMatrixMixer::MOD12_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(78.0, 52.0)), module, StereoMatrixMixer::MOD22_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(112.0, 52.0)), module, StereoMatrixMixer::MOD32_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(146.0, 52.0)), module, StereoMatrixMixer::MOD42_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.0, 66.0)), module, StereoMatrixMixer::L3_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.0, 78.0)), module, StereoMatrixMixer::R3_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(44.0, 78.0)), module, StereoMatrixMixer::MOD13_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(78.0, 78.0)), module, StereoMatrixMixer::MOD23_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(112.0, 78.0)), module, StereoMatrixMixer::MOD33_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(146.0, 78.0)), module, StereoMatrixMixer::MOD43_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.0, 92.0)), module, StereoMatrixMixer::L4_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.0, 104.0)), module, StereoMatrixMixer::R4_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(44.0, 104.0)), module, StereoMatrixMixer::MOD14_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(78.0, 104.0)), module, StereoMatrixMixer::MOD24_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(112.0, 104.0)), module, StereoMatrixMixer::MOD34_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(146.0, 104.0)), module, StereoMatrixMixer::MOD44_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(27.5, 118.0)), module, StereoMatrixMixer::OL1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(39.5, 118.0)), module, StereoMatrixMixer::OR1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(61.588, 118.002)), module, StereoMatrixMixer::OL2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(73.588, 118.0)), module, StereoMatrixMixer::OR2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(95.5, 118.0)), module, StereoMatrixMixer::OL3_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(107.5, 118.0)), module, StereoMatrixMixer::OR3_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(132.034, 118.0)), module, StereoMatrixMixer::OL4_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(144.0, 118.0)), module, StereoMatrixMixer::OR4_OUTPUT));

		addChild(createLightCentered<SmallLight<RedGreenBlueLight>>(mm2px(Vec(20.0, 28.0)), module, StereoMatrixMixer::LED11_LIGHT));
		addChild(createLightCentered<SmallLight<RedGreenBlueLight>>(mm2px(Vec(54.0, 28.0)), module, StereoMatrixMixer::LED21_LIGHT));
		addChild(createLightCentered<SmallLight<RedGreenBlueLight>>(mm2px(Vec(88.0, 28.0)), module, StereoMatrixMixer::LED31_LIGHT));
		addChild(createLightCentered<SmallLight<RedGreenBlueLight>>(mm2px(Vec(122.0, 28.0)), module, StereoMatrixMixer::LED41_LIGHT));
		addChild(createLightCentered<SmallLight<RedGreenBlueLight>>(mm2px(Vec(20.0, 54.0)), module, StereoMatrixMixer::LED12_LIGHT));
		addChild(createLightCentered<SmallLight<RedGreenBlueLight>>(mm2px(Vec(54.0, 54.0)), module, StereoMatrixMixer::LED22_LIGHT));
		addChild(createLightCentered<SmallLight<RedGreenBlueLight>>(mm2px(Vec(88.0, 54.0)), module, StereoMatrixMixer::LED32_LIGHT));
		addChild(createLightCentered<SmallLight<RedGreenBlueLight>>(mm2px(Vec(122.0, 54.0)), module, StereoMatrixMixer::LED42_LIGHT));
		addChild(createLightCentered<SmallLight<RedGreenBlueLight>>(mm2px(Vec(20.0, 80.0)), module, StereoMatrixMixer::LED13_LIGHT));
		addChild(createLightCentered<SmallLight<RedGreenBlueLight>>(mm2px(Vec(54.0, 80.0)), module, StereoMatrixMixer::LED23_LIGHT));
		addChild(createLightCentered<SmallLight<RedGreenBlueLight>>(mm2px(Vec(88.0, 80.0)), module, StereoMatrixMixer::LED33_LIGHT));
		addChild(createLightCentered<SmallLight<RedGreenBlueLight>>(mm2px(Vec(122.0, 80.0)), module, StereoMatrixMixer::LED43_LIGHT));
		addChild(createLightCentered<SmallLight<RedGreenBlueLight>>(mm2px(Vec(20.0, 106.0)), module, StereoMatrixMixer::LED14_LIGHT));
		addChild(createLightCentered<SmallLight<RedGreenBlueLight>>(mm2px(Vec(54.0, 106.0)), module, StereoMatrixMixer::LED24_LIGHT));
		addChild(createLightCentered<SmallLight<RedGreenBlueLight>>(mm2px(Vec(88.0, 106.0)), module, StereoMatrixMixer::LED34_LIGHT));
		addChild(createLightCentered<SmallLight<RedGreenBlueLight>>(mm2px(Vec(122.0, 106.0)), module, StereoMatrixMixer::LED44_LIGHT));
	}
};


Model* modelStereoMatrixMixer = createModel<StereoMatrixMixer, StereoMatrixMixerWidget>("StereoMatrixMixer");