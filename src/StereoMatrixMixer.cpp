#include "plugin.hpp"

float ternary(const bool cond, const float a, const float b) {
	return cond ? a : b;
}

float voltageToBrightness(const float leftValue, const float rightValue) {
	return (leftValue + rightValue + 10.f) / 20.f;
}

struct StereoMatrixMixer : Module {
	enum ParamId {
		MIX11_PARAM,
		MIX21_PARAM,
		MIX31_PARAM,
		MIX41_PARAM,
		MIX12_PARAM,
		MIX22_PARAM,
		MIX32_PARAM,
		MIX42_PARAM,
		MIX13_PARAM,
		MIX23_PARAM,
		MIX33_PARAM,
		MIX43_PARAM,
		MIX14_PARAM,
		MIX24_PARAM,
		MIX34_PARAM,
		MIX44_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		L1_INPUT,
		R1_INPUT,
		L2_INPUT,
		R2_INPUT,
		L3_INPUT,
		R3_INPUT,
		L4_INPUT,
		R4_INPUT,
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
		LED1_LIGHT,
		LED2_LIGHT,
		LED3_LIGHT,
		LED4_LIGHT,
		LED5_LIGHT,
		LED6_LIGHT,
		LED7_LIGHT,
		LED8_LIGHT,
		LED9_LIGHT,
		LED10_LIGHT,
		LED11_LIGHT,
		LED12_LIGHT,
		LED13_LIGHT,
		LED14_LIGHT,
		LED15_LIGHT,
		LED16_LIGHT,
		LIGHTS_LEN
	};

	StereoMatrixMixer() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(MIX11_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MIX21_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MIX31_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MIX41_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MIX12_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MIX22_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MIX32_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MIX42_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MIX13_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MIX23_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MIX33_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MIX43_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MIX14_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MIX24_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MIX34_PARAM, 0.f, 1.f, 0.f, "");
		configParam(MIX44_PARAM, 0.f, 1.f, 0.f, "");
		configInput(L1_INPUT, "");
		configInput(R1_INPUT, "");
		configInput(L2_INPUT, "");
		configInput(R2_INPUT, "");
		configInput(L3_INPUT, "");
		configInput(R3_INPUT, "");
		configInput(L4_INPUT, "");
		configInput(R4_INPUT, "");
		configOutput(OL1_OUTPUT, "");
		configOutput(OR1_OUTPUT, "");
		configOutput(OL2_OUTPUT, "");
		configOutput(OR2_OUTPUT, "");
		configOutput(OL3_OUTPUT, "");
		configOutput(OR3_OUTPUT, "");
		configOutput(OL4_OUTPUT, "");
		configOutput(OR4_OUTPUT, "");
	}

	void process(const ProcessArgs& args) override {
		float outL[4] = {0.f, 0.f, 0.f, 0.f};
		float outR[4] = {0.f, 0.f, 0.f, 0.f};

		// read input
		const float inL[4] = {
			inputs[L1_INPUT].getVoltage(),
			inputs[L2_INPUT].getVoltage(),
			inputs[L3_INPUT].getVoltage(),
			inputs[L4_INPUT].getVoltage()
		};

		const float inR[4] = {
			ternary(inputs[R1_INPUT].isConnected(), inputs[R1_INPUT].getVoltage(), inL[0]),
			ternary(inputs[R2_INPUT].isConnected(), inputs[R2_INPUT].getVoltage(), inL[1]),
			ternary(inputs[R3_INPUT].isConnected(), inputs[R3_INPUT].getVoltage(), inL[2]),
			ternary(inputs[R4_INPUT].isConnected(), inputs[R4_INPUT].getVoltage(), inL[3])
		};

		constexpr int mixParams[4][4] = {
			{MIX11_PARAM, MIX12_PARAM, MIX13_PARAM, MIX14_PARAM},
			{MIX21_PARAM, MIX22_PARAM, MIX23_PARAM, MIX24_PARAM},
			{MIX31_PARAM, MIX32_PARAM, MIX33_PARAM, MIX34_PARAM},
			{MIX41_PARAM, MIX42_PARAM, MIX43_PARAM, MIX44_PARAM}
		};

		constexpr int mixLights[4][4] = {
			{LED1_LIGHT, LED2_LIGHT, LED3_LIGHT, LED4_LIGHT},
			{LED5_LIGHT, LED6_LIGHT, LED7_LIGHT, LED8_LIGHT},
			{LED9_LIGHT, LED10_LIGHT, LED11_LIGHT, LED12_LIGHT},
			{LED13_LIGHT, LED14_LIGHT, LED15_LIGHT, LED16_LIGHT}
		};

		for (int i = 0; i < 4; i++) {
			float mixL = 0.f;
			float mixR = 0.f;

			for (int j = 0; j < 4; j++) {
				mixL += params[mixParams[i][j]].getValue() * inL[j];
				mixR += params[mixParams[i][j]].getValue() * inR[j];
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

		// set the lights
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				const float brightness = clamp(voltageToBrightness(outL[i], outR[i]), 0.f, 1.f) * params[mixParams[i][j]].getValue();
				lights[mixLights[j][i]].setBrightness(brightness);
			}
		}
	}
};


struct StereoMatrixMixerWidget : ModuleWidget {
	explicit StereoMatrixMixerWidget(StereoMatrixMixer* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/StereoMatrixMixer.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(26.987, 19.899)), module, StereoMatrixMixer::MIX11_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(52.987, 19.899)), module, StereoMatrixMixer::MIX21_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(78.987, 19.899)), module, StereoMatrixMixer::MIX31_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(104.987, 19.899)), module, StereoMatrixMixer::MIX41_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(26.987, 45.899)), module, StereoMatrixMixer::MIX12_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(52.987, 45.899)), module, StereoMatrixMixer::MIX22_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(78.987, 45.899)), module, StereoMatrixMixer::MIX32_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(104.987, 45.899)), module, StereoMatrixMixer::MIX42_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(26.987, 71.899)), module, StereoMatrixMixer::MIX13_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(52.987, 71.899)), module, StereoMatrixMixer::MIX23_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(78.987, 71.899)), module, StereoMatrixMixer::MIX33_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(104.987, 71.899)), module, StereoMatrixMixer::MIX43_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(26.987, 97.899)), module, StereoMatrixMixer::MIX14_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(52.987, 97.899)), module, StereoMatrixMixer::MIX24_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(78.987, 97.899)), module, StereoMatrixMixer::MIX34_PARAM));
		addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(104.987, 97.899)), module, StereoMatrixMixer::MIX44_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.987, 13.899)), module, StereoMatrixMixer::L1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.987, 25.899)), module, StereoMatrixMixer::R1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.987, 39.899)), module, StereoMatrixMixer::L2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.987, 51.899)), module, StereoMatrixMixer::R2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.987, 65.899)), module, StereoMatrixMixer::L3_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.987, 77.899)), module, StereoMatrixMixer::R3_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.987, 91.899)), module, StereoMatrixMixer::L4_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.987, 103.899)), module, StereoMatrixMixer::R4_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.933, 113.901)), module, StereoMatrixMixer::OL1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(32.933, 113.901)), module, StereoMatrixMixer::OR1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(46.987, 113.901)), module, StereoMatrixMixer::OL2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(58.987, 113.899)), module, StereoMatrixMixer::OR2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(72.933, 113.901)), module, StereoMatrixMixer::OL3_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(84.933, 113.901)), module, StereoMatrixMixer::OR3_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(99.001, 113.899)), module, StereoMatrixMixer::OL4_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(110.967, 113.899)), module, StereoMatrixMixer::OR4_OUTPUT));

		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(34.987, 27.899)), module, StereoMatrixMixer::LED1_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(60.987, 27.899)), module, StereoMatrixMixer::LED2_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(86.987, 27.899)), module, StereoMatrixMixer::LED3_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(112.987, 27.899)), module, StereoMatrixMixer::LED4_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(35.487, 53.399)), module, StereoMatrixMixer::LED5_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(60.987, 53.899)), module, StereoMatrixMixer::LED6_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(86.987, 53.899)), module, StereoMatrixMixer::LED7_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(112.987, 53.899)), module, StereoMatrixMixer::LED8_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(34.987, 79.899)), module, StereoMatrixMixer::LED9_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(60.987, 79.899)), module, StereoMatrixMixer::LED10_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(86.987, 79.899)), module, StereoMatrixMixer::LED11_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(112.987, 79.899)), module, StereoMatrixMixer::LED12_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(34.987, 105.899)), module, StereoMatrixMixer::LED13_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(60.987, 105.899)), module, StereoMatrixMixer::LED14_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(86.987, 105.899)), module, StereoMatrixMixer::LED15_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(112.987, 105.899)), module, StereoMatrixMixer::LED16_LIGHT));
	}
};


Model* modelStereoMatrixMixer = createModel<StereoMatrixMixer, StereoMatrixMixerWidget>("StereoMatrixMixer");