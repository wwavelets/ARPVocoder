/*
  ==============================================================================

    Plugin.cpp
    Created: 12 Oct 2024 1:01:53pm
    Author:  erich

  ==============================================================================
*/

#include "Plugin.h"

Plugin::Plugin(VocoderParameters parameters, juce::AudioProcessor* head) : parameters(parameters)
{
    bandData.resize(128);
    bandSliders.resize(128);
    parameters.bandData = &bandData;
    parameters.carrierMapping = &carrierMap;
    parameters.modulatorMapping = &modulatorMap;
    processor = std::make_unique<VocoderProcessor>(parameters, head);


    carrierMap.setLineRenderer(75, 60, 275, 260);
    modulatorMap.setLineRenderer(750, 60, 950, 260);


    numBands.setCallBack([&](int val) {
        std::vector<int> nums = { 3, 4, 5, 6, 8, 12, 16, 20, 24, 32, 40 };
        if (val < static_cast<int>(nums.size())) {
            processor->setBandWidth(pow(processor->getSampleRate() / 2.0 / processor->getMinFreq(), 1.0 / (nums[val] + 0.01)));
            updateSliders();
            return;
        }
        val -= static_cast<int>(nums.size());
        std::vector<int> nums2 = { 6, 8, 10, 12 };
        processor->setBandWidth(pow(2.0, 1.0 / nums2[val]));
        updateSliders();
    });
    numBands.setChoices({ "3", "4", "5", "6", "8", "12", "16", "20", "24", "32", "40", "6 / oct", "8 / oct", "10 / oct", "12 / oct" });


    editChoice.setCallBack([&](int val) {
        if (val == 0) viewingMode = Default;
        if (val == 1) viewingMode = PreGain;
        if (val == 2) viewingMode = PostGain;
        if (val == 3) viewingMode = Delay;
        if (val == 4) viewingMode = ModGain;
        updateSliders();
    });
    editChoice.setChoices({ "Default", "Pre-Gain", "Post-Gain", "Delay", "Mod. Gain" });



    numRepeat.setCallBack([&](int val) {
        if (val == 0)countRepeat = 1000;
        else countRepeat = val;
        updateSliders();
    });
    numRepeat.setChoices({ "All", "1", "2", "3","4","5","6","7","8","9","10","11","12" });

    filterMode.setCallBack([&](int val) {
        if (val == 0) {
            numBands.setChoices({ "3", "4", "5", "6", "8", "12", "16", "20", "24", "32", "40", "6 / oct", "8 / oct", "10 / oct", "12 / oct" }, numBands.getVal());
        }
        else if(val == 1) {
            numBands.setChoices({ "3", "4", "5", "6", "8", "12", "16", "20", "24", "32", "40" }, std::min(10, numBands.getVal()));
        }

        if (val == 0) {
            processor->setIIR();
        }
        else if (val == 1) {
            processor->setFIR();
        }
    });
    filterMode.setChoices({ "Default", "Lin. Phase" });

    modulatorType.setChoices({ "Self", "Sidechain" });

    attackSlider = std::make_unique<RadialSlider>(parameters.attack);
    releaseSlider = std::make_unique<RadialSlider>(parameters.release);
    formantSlider = std::make_unique<RadialSlider>(parameters.formant);
    carrierBandSlider = std::make_unique<RadialSlider>(parameters.carrierBandWidth);
    modulatorBandSlider = std::make_unique<RadialSlider>(parameters.modulatorBandWidth);
    mixSlider = std::make_unique<RadialSlider>(parameters.mix);

    {
        std::lock_guard<std::mutex> lock(m);
        initialized = true;
    }
    cv.notify_one();
}

Plugin::~Plugin() {

}

void Plugin::init(OpenGLWrapper& opengl)
{
    //initialize stuff here
    opengl.init();
    TextRenderer::initTextures(opengl);

    carrierMap.init(opengl);
    carrierMap.setColor(juce::Colours::darkred);
    modulatorMap.init(opengl);
    modulatorMap.setColor(juce::Colours::darkred);

    //UI
    for (int i = -60; i <= 0; i += 10) {
        TextRenderer tmp;
        tmp.setText(opengl, std::to_string(i));
        tmp.setSize(0.225f);
        tmp.setPos(710 + 240 * pow(pow(0.5f, i / -6.0206f), 0.333f), 40);
        tmp.setColor(juce::Colours::darkred);
        markers.push_back(std::move(tmp));
        if (i == -60)i += 10;
    }

    for (int i = -60; i <= 0; i += 10) {
        TextRenderer tmp;
        tmp.setText(opengl, std::to_string(i));
        tmp.setSize(0.225f);
        tmp.setPos((i == 0) * 13 + 705, 50 + 200 * pow(pow(0.5f, i / -6.0206f), 0.333f));
        tmp.setColor(juce::Colours::darkred);
        markers.push_back(std::move(tmp));
        if (i == -60)i += 10;
    }

    for (int i = -60; i <= 0; i += 10) {
        TextRenderer tmp;
        tmp.setText(opengl, std::to_string(i));
        tmp.setSize(0.225f);
        tmp.setPos(35 + 240 * pow(pow(0.5f, i / -6.0206f), 0.333f), 40);
        tmp.setColor(juce::Colours::darkred);
        markers.push_back(std::move(tmp));
        if (i == -60)i += 10;
    }


    for (int i = -60; i <= 0; i += 10) {
        TextRenderer tmp;
        tmp.setText(opengl, std::to_string(i));
        tmp.setSize(0.225f);
        tmp.setPos((i == 0) * 13 + 30, 50 + 200 * pow(pow(0.5f, i / -6.0206f), 0.333f));
        tmp.setColor(juce::Colours::darkred);
        markers.push_back(std::move(tmp));
        if (i == -60)i += 10;
    }

    dynamics.setText(opengl, "Dynamics", 93 + 25, 290, 0.3f);
    dynamics.setColor(juce::Colours::darkred);

    modulation.setText(opengl, "Modulation", 793, 290, 0.3f);
    modulation.setColor(juce::Colours::darkred);


    bars.resize(128); // max number?
    for (auto& bar : bars) {
        bar.init(opengl);
        bar.setColor(juce::Colours::darkred);
    }

    view.setText(opengl, "View:  ", 45, 569, 0.3f);
    view.setColor(juce::Colours::darkred);
    editChoice.init(opengl);
    editChoice.setDropDown(opengl, 45 + view.getLength(), 575, 0.3f);
    editChoice.setColor(juce::Colours::darkred, juce::Colours::pink);


    bands.setText(opengl, "Bands:  ", 230, 569, 0.3f);
    bands.setColor(juce::Colours::darkred);
    numBands.init(opengl);
    numBands.setDropDown(opengl, 230 + bands.getLength(), 575, 0.3f);
    numBands.setColor(juce::Colours::darkred, juce::Colours::pink);

    repeat.setText(opengl, "Repeat:  ", 405, 569, 0.3f);
    repeat.setColor(juce::Colours::darkred);
    numRepeat.init(opengl);
    numRepeat.setDropDown(opengl, 405 + repeat.getLength(), 575, 0.3f);
    numRepeat.setColor(juce::Colours::darkred, juce::Colours::pink);

    mode.setText(opengl, "Mode:  ", 545, 569, 0.3f);
    mode.setColor(juce::Colours::darkred);
    filterMode.init(opengl);
    filterMode.setDropDown(opengl, 545 + mode.getLength(), 575, 0.3f);
    filterMode.setColor(juce::Colours::darkred, juce::Colours::pink);


    modulator.setText(opengl, "Modulator:  ", 745, 569, 0.3f);
    modulator.setColor(juce::Colours::darkred);
    modulatorType.init(opengl);
    modulatorType.setDropDown(opengl, 745 + modulator.getLength(), 575, 0.3f);
    modulatorType.setColor(juce::Colours::darkred, juce::Colours::pink);

    for (auto& slider : bandSliders) {
        slider.init(opengl);
        slider.setColor(juce::Colours::darkred, juce::Colours::darkred.withAlpha(0.25f));
    }

    attackText.setText(opengl, "Attack", 355, 185, 0.225f);
    attackText.setColor(juce::Colours::darkred);
    attackSlider->init(opengl);
    attackSlider->setColor(juce::Colours::darkred);
    attackSlider->setSlider(375, 220, 30.0f);

    releaseText.setText(opengl, "Release", 472, 185, 0.225f);
    releaseText.setColor(juce::Colours::darkred);
    releaseSlider->init(opengl);
    releaseSlider->setColor(juce::Colours::darkred);
    releaseSlider->setSlider(500, 220, 30.0f);

    formantText.setText(opengl, "Formant", 598, 185, 0.225f);
    formantText.setColor(juce::Colours::darkred);
    formantSlider->init(opengl);
    formantSlider->setColor(juce::Colours::darkred);
    formantSlider->setSlider(625, 220, 30.0f);

    carrierText.setText(opengl, "Carrier BW", 337, 55, 0.225f);
    carrierText.setColor(juce::Colours::darkred);
    carrierBandSlider->init(opengl);
    carrierBandSlider->setColor(juce::Colours::darkred);
    carrierBandSlider->setSlider(375, 90, 30.0f);

    modulatorText.setText(opengl, "Modulator BW", 453, 55, 0.225f);
    modulatorText.setColor(juce::Colours::darkred);
    modulatorBandSlider->init(opengl);
    modulatorBandSlider->setColor(juce::Colours::darkred);
    modulatorBandSlider->setSlider(500, 90, 30.0f);

    mixText.setText(opengl, "Mix", 612, 55, 0.225f);
    mixText.setColor(juce::Colours::darkred);
    mixSlider->init(opengl);
    mixSlider->setColor(juce::Colours::darkred);
    mixSlider->setSlider(625, 90, 30.0f);
    updateSliders();

    FIRUnusedParametersRectangle.init(opengl);
    FIRUnusedParametersRectangle.setColor(juce::Colours::pink.withAlpha(0.75f));
}

void Plugin::updateSliders() {
    numBars = processor->getNumBands(); //lmao

    for (int i = 0; i < std::min(numBars, countRepeat); i++) {
#define setSliders(_mode, _name) \
        if (viewingMode == _mode) { \
            float tmp = bandData[i]._name; \
            bandSliders[i].setCallBack([&](float val, int id) { \
                for (int j = id; j < bandData.size(); j += countRepeat) { \
                    bandData[j]._name = val; \
                } \
            }, i); \
            bandSliders[i].setVal(tmp); \
        }

        setSliders(PreGain, gain);
        setSliders(PostGain, postGain);
        setSliders(ModGain, modulatorGain);
        setSliders(Delay, delay);
#undef setSliders
    }

    numBars = std::min(numBars, countRepeat);

    float width = 900.0 / numBars;
    float cur = 50;
    for (int i = 0; i < numBars; i++) {
        bandSliders[i].setSlider(cur, 315, cur + width, 315 + 1 * 200);
        //bandSliders[i].setVal(0.5f);
        cur += width;
    }
    DBG("bars " << numBars << "\n");
}


void Plugin::update(OpenGLWrapper&opengl, InputWrapper&inputs)
{
    //juce::gl::glEnable(juce::gl::GL_BLEND);
    //juce::gl::glBlendFunc(juce::gl::GL_SRC_ALPHA, juce::gl::GL_ONE_MINUS_SRC_ALPHA);
    //juce::OpenGLHelpers::clear(juce::Colours::pink);
    //inputs.updateInputs(getMouseXYRelative().x * 1000 / getWidth(), 600 - getMouseXYRelative().y * 600 / getHeight());



    carrierMap.update(opengl, inputs, 0);
    modulatorMap.update(opengl, inputs, 0);

    for (auto& x : markers) x.upd(opengl);

    dynamics.upd(opengl);
    modulation.upd(opengl);

    juce::Rectangle<int> boundingBox(50, 315, 900, 200);
    if (boundingBox.contains(inputs.mouseX(), inputs.mouseY()) && inputs.canFocus()) {
        inputs.setFocused();
        interfaceFocused = true;
    }
    if (interfaceFocused == true && (!inputs.mouseDown())) {
        interfaceFocused = false;
        inputs.setNotFocused();
    }
    if (viewingMode == Default) {
        if (interfaceFocused == true) {
            interfaceFocused = false;
            inputs.setNotFocused();
        }
        auto envelope = processor->getEnvelope();
        int numBars = envelope.size();
        float width = 900.0 / numBars;
        float cur = 50;
        for (int i = 0; i < numBars; i++) {
            //bars[i].setRect(opengl, cur, 315, cur + width, 315 + std::max(0.0f, 20.0f * log10(envelope[i])+60)*5);
            bars[i].setRect(opengl, cur, 315, cur + width, 315 + envelope[i] * 200);
            bars[i].update(opengl);
            cur += width;
        }
    }
    else {
        for (int i = 0; i < numBars; i++) {
            bandSliders[i].renderSlider(opengl);
            if (interfaceFocused) {
                bandSliders[i].updateInputs(inputs);
            }
        }
    }

    attackSlider->update(opengl, inputs);
    releaseSlider->update(opengl, inputs);
    formantSlider->update(opengl, inputs);
    carrierBandSlider->update(opengl, inputs);
    modulatorBandSlider->update(opengl, inputs);
    mixSlider->update(opengl, inputs);

    attackText.upd(opengl);
    releaseText.upd(opengl);
    formantText.upd(opengl);
    carrierText.upd(opengl);
    modulatorText.upd(opengl);
    mixText.upd(opengl);

    if (filterMode.getVal() == 1) {
        FIRUnusedParametersRectangle.setRect(opengl, 325, 20, 550, 150);
        FIRUnusedParametersRectangle.update(opengl);
    }
    view.upd(opengl);
    editChoice.update(opengl, inputs);
    bands.upd(opengl);
    numBands.update(opengl, inputs);
    repeat.upd(opengl);
    numRepeat.update(opengl, inputs);
    mode.upd(opengl);
    filterMode.update(opengl, inputs);
    modulator.upd(opengl);
    modulatorType.update(opengl, inputs);

}

void Plugin::destroy(OpenGLWrapper&opengl)
{
    //destroying stuff goes here
    carrierMap.destroy(opengl);
    modulatorMap.destroy(opengl);
    formantSlider->destroy(opengl);

    for (auto& x : markers) x.destroy(opengl);
    markers.clear();


    dynamics.destroy(opengl);
    modulation.destroy(opengl);

    for (auto& bar : bars) bar.destroy(opengl);
    bars.clear();

    editChoice.destroy(opengl);
    numBands.destroy(opengl);
    numRepeat.destroy(opengl);
    filterMode.destroy(opengl);
    modulatorType.destroy(opengl);


    view.destroy(opengl);
    bands.destroy(opengl);
    repeat.destroy(opengl);
    mode.destroy(opengl);
    modulator.destroy(opengl);
    opengl.destroy();

    carrierMap.destroy(opengl);
    modulatorMap.destroy(opengl);


    for (auto& slider : bandSliders) slider.destroy(opengl);
    //bandSliders.clear();


    attackSlider->destroy(opengl);
    attackText.destroy(opengl);
    releaseSlider->destroy(opengl);
    releaseText.destroy(opengl);
    formantSlider->destroy(opengl);
    formantText.destroy(opengl);
    carrierBandSlider->destroy(opengl);
    carrierText.destroy(opengl);
    modulatorBandSlider->destroy(opengl);
    modulatorText.destroy(opengl);
    mixSlider->destroy(opengl);
    mixText.destroy(opengl);

    FIRUnusedParametersRectangle.destroy(opengl);

    std::vector<RectSlider> bandSliders; // bruhh 
    bool interfaceFocused = false;
}


void Plugin::loadPlugin(std::stringstream& in) {

    std::unique_lock<std::mutex> lock(m);
    cv.wait(lock, [this] {return this->initialized; });

    carrierMap.load(in);
    modulatorMap.load(in);
    for (auto& x : bandData) in >> x.delay >> x.envelope >> x.gain >> x.modulatorGain >> x.postGain;
    int val; 
    in >> val; editChoice.setVal(val);
    in >> val; numBands.setVal(val);
    in >> val; numRepeat.setVal(val);
    in >> val; filterMode.setVal(val);
    in >> val; modulatorType.setVal(val);

    float fval;
    in >> fval;  (*(parameters.attack)) = fval;
    in >> fval;  (*(parameters.release)) = fval;
    in >> fval;  (*(parameters.formant)) = fval;
    in >> fval;  (*(parameters.carrierBandWidth)) = fval;
    in >> fval;  (*(parameters.modulatorBandWidth)) = fval;
    in >> fval;  (*(parameters.mix)) = fval;
}
void Plugin::savePlugin(std::stringstream& out) {
    carrierMap.save(out);
    modulatorMap.save(out);
    for (auto& x : bandData) out << x.delay << " " << x.envelope << " " << x.gain << " " << x.modulatorGain << " " << x.postGain << "\n";

    out << editChoice.getVal() << "\n";
    out << numBands.getVal() << "\n";
    out << numRepeat.getVal() << "\n";
    out << filterMode.getVal() << "\n";
    out << modulatorType.getVal() << "\n";

    out << std::fixed << std::setprecision(8) << (*(parameters.attack)).get() << "\n";
    out << std::fixed << std::setprecision(8) << (*(parameters.release)).get() << "\n";
    out << std::fixed << std::setprecision(8) << (*(parameters.formant)).get() << "\n";
    out << std::fixed << std::setprecision(8) << (*(parameters.carrierBandWidth)).get() << "\n";
    out << std::fixed << std::setprecision(8) << (*(parameters.modulatorBandWidth)).get() << "\n";
    out << std::fixed << std::setprecision(8) << (*(parameters.mix)).get() << "\n";
}