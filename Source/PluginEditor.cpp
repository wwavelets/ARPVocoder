/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewProjectAudioProcessorEditor::NewProjectAudioProcessorEditor (NewProjectAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (750, 450);
    setResizable(true, false);

    addAndMakeVisible(interface);
    interface.setSize(750, 450);
    interface.init(p.plugin.get());
    interface.toBack();

    //addAndMakeVisible(&button);
    //button.setButtonText("play/pause");
    //button.onClick = [&]() {
    //    if (button.getToggleState()) {
    //        audioProcessor.curState = audioProcessor.start;
    //    }
    //    else {
    //        audioProcessor.curState = audioProcessor.stop;
    //    }
    //};

    //addAndMakeVisible(&chooseButton);
    //chooseButton.setButtonText("choose file");

    //chooser = std::make_unique<juce::FileChooser>("Choose a file", juce::File{}, "*.wav,*.mp3");
    //chooseButton.onClick = [&]() {
    //    chooser->launchAsync(juce::FileBrowserComponent::openMode
    //        | juce::FileBrowserComponent::canSelectFiles, [&](const juce::FileChooser& fc) {
    //            juce::File file = fc.getResult();
    //            if (file == juce::File{}) {
    //                return;
    //            }
    //            audioProcessor.loadFile(file);
    //        });
    //};

}

NewProjectAudioProcessorEditor::~NewProjectAudioProcessorEditor()
{
    interface.destroy();
}

//==============================================================================
void NewProjectAudioProcessorEditor::paint (juce::Graphics& g)
{
    //imagine rendering things
}

void NewProjectAudioProcessorEditor::resized()
{
    
    int w = getWidth();
    int h = getHeight();
    int alpha = std::max(100, std::min(w, h*5/3));
    setSize(alpha, alpha*3/5);
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    //button.setBounds(0, 0, 100, 50);
   // chooseButton.setBounds(100, 0, 100, 50);
    interface.setBounds(0, 0, alpha, alpha*0.6f);
}
