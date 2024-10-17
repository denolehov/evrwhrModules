#include "plugin.hpp"


struct Phoenix : Module {
	enum ParamId {
		HIT_STRENGTH_PARAM,
		RISE_SPEED_PARAM,
		HIT_STRENGTH_CV_PARAM,
		RISE_CV_PARAM,
		WEAKEN_MODE_PARAM,
		HIT_PARAM,
		LIN_EXP_PARAM,
		ATT_MODE_PARAM,
		INVERT_PARAM,
		OUTPUT_RANGE_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		HIT_STRENGTH_CV_INPUT,
		RISE_CV_INPUT,
		HIT_INPUT,
		INVERT_INPUT,
		MAIN_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		FALLEN_OUTPUT,
		RISEN_OUTPUT,
		MAIN_OUTPUT,
		AUX_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	Phoenix() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(HIT_STRENGTH_PARAM, 0.f, 1.f, 0.f, "");
		configParam(RISE_SPEED_PARAM, 0.f, 1.f, 0.f, "");
		configParam(HIT_STRENGTH_CV_PARAM, 0.f, 1.f, 0.f, "");
		configParam(RISE_CV_PARAM, 0.f, 1.f, 0.f, "");
		configParam(WEAKEN_MODE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(HIT_PARAM, 0.f, 1.f, 0.f, "");
		configParam(LIN_EXP_PARAM, 0.f, 1.f, 0.f, "");
		configParam(ATT_MODE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(INVERT_PARAM, 0.f, 1.f, 0.f, "");
		configParam(OUTPUT_RANGE_PARAM, 0.f, 1.f, 0.f, "");
		configInput(HIT_STRENGTH_CV_INPUT, "");
		configInput(RISE_CV_INPUT, "");
		configInput(HIT_INPUT, "");
		configInput(INVERT_INPUT, "");
		configInput(MAIN_INPUT, "");
		configOutput(FALLEN_OUTPUT, "");
		configOutput(RISEN_OUTPUT, "");
		configOutput(MAIN_OUTPUT, "");
		configOutput(AUX_OUTPUT, "");
	}

	void process(const ProcessArgs& args) override {
	}
};


struct PhoenixWidget : ModuleWidget {
	PhoenixWidget(Phoenix* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Phoenix.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundHugeBlackKnob>(mm2px(Vec(40.348, 17.691)), module, Phoenix::HIT_STRENGTH_PARAM));
		addParam(createParamCentered<RoundHugeBlackKnob>(mm2px(Vec(21.375, 24.017)), module, Phoenix::RISE_SPEED_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(46.038, 31.595)), module, Phoenix::HIT_STRENGTH_CV_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(25.168, 38.554)), module, Phoenix::RISE_CV_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(41.471, 52.675)), module, Phoenix::WEAKEN_MODE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(49.201, 75.857)), module, Phoenix::HIT_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(65.722, 77.982)), module, Phoenix::LIN_EXP_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(57.265, 81.07)), module, Phoenix::ATT_MODE_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(45.881, 84.866)), module, Phoenix::INVERT_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(53.963, 90.139)), module, Phoenix::OUTPUT_RANGE_PARAM));

		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(48.645, 42.574)), module, Phoenix::HIT_STRENGTH_CV_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(29.672, 48.901)), module, Phoenix::RISE_CV_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(44.694, 62.339)), module, Phoenix::HIT_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(37.263, 87.472)), module, Phoenix::INVERT_INPUT));
		addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(57.819, 101.7)), module, Phoenix::MAIN_INPUT));

		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(57.344, 68.662)), module, Phoenix::FALLEN_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(38.371, 74.989)), module, Phoenix::RISEN_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(65.723, 90.632)), module, Phoenix::MAIN_OUTPUT));
		addOutput(createOutputCentered<DarkPJ301MPort>(mm2px(Vec(44.853, 97.591)), module, Phoenix::AUX_OUTPUT));
	}
};


Model* modelPhoenix = createModel<Phoenix, PhoenixWidget>("Phoenix");