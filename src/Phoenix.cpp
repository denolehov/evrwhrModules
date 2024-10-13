#include "plugin.hpp"


struct Phoenix : Module {
	enum ParamId {
		PARAMS_LEN
	};
	enum InputId {
		SIGNAL_INPUT,
		TRIGGER_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	// TODO: Save / Load these
	float targetBaseline = 0.f;
	float baseline = 1.f;

	dsp::SchmittTrigger trigger;
	dsp::ExponentialSlewLimiter slewLimiter;

	// TODO: SIGNAL -> MAIN
	// TODO: OUT -> MAIN

	Phoenix() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configInput(SIGNAL_INPUT, "");
		configInput(TRIGGER_INPUT, "");
		configOutput(OUT_OUTPUT, "");
	}

	void process(const ProcessArgs& args) override {
		// get the input voltage
		// TODO: Polyphony.
		// TODO: Configurable input/output ranges (e.g. -5V to 5V, 0V to 10V, etc.)
		float signal = getInput(SIGNAL_INPUT).getVoltage();

		// attenuate it according to the triggers
		if (trigger.process(getInput(TRIGGER_INPUT).getVoltage(), 0.1f, 2.f)) {
			// TODO: Add a "slow" mode where the baseline is nudged by an random amount (up to a user-defined limit).
			baseline -= .1f; // TODO: Make this a parameter
			baseline = clamp(baseline, 0.f, 1.f);
		}

		// This is for attenuation mode.
		// float out = signal * baseline;

		// This is for "nudge" (offset) mode.
		float y_max = rescale(baseline, 0.f, 1.f, -10.f, 10.f);
		float out = rescale(signal, -10.f, 10.f, -10.f, y_max);

		// output it
		getOutput(OUT_OUTPUT).setVoltage(out);
	}
};


struct PhoenixWidget : ModuleWidget {
	PhoenixWidget(Phoenix* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Phoenix.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 15.25)), module, Phoenix::SIGNAL_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 31.92)), module, Phoenix::TRIGGER_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 48.25)), module, Phoenix::OUT_OUTPUT));
	}
};


Model* modelPhoenix = createModel<Phoenix, PhoenixWidget>("Phoenix");