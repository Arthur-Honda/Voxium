#pragma once

#include <JuceHeader.h>

// Tema visual do Voxium: dark/minimalista, com um unico accent violeta
// (no estilo de plugins como FabFilter/Soundtoys). Centraliza a paleta
// de cores aqui, pra qualquer componente novo que a gente adicionar
// depois (knobs, meters, etc) puxar as mesmas cores em vez de valores
// soltos espalhados pelo codigo.
namespace VoxiumColours
{
	static const juce::Colour background{ 0xff17171b };
	static const juce::Colour panel{ 0xff202027 };
	static const juce::Colour border{ 0xff2e2e36 };
	static const juce::Colour textPrimary{ 0xffe8e6f0 };
	static const juce::Colour textSecondary{ 0xff8a8894 };
	static const juce::Colour accent{ 0xff8b7cf6 };
	static const juce::Colour accentHover{ 0xffa79af9 };
}

class VoxiumLookAndFeel : public juce::LookAndFeel_V4
{
public:
	VoxiumLookAndFeel()
	{
		setColour(juce::ResizableWindow::backgroundColourId, VoxiumColours::background);

		setColour(juce::Label::textColourId, VoxiumColours::textPrimary);

		setColour(juce::ComboBox::backgroundColourId, VoxiumColours::panel);
		setColour(juce::ComboBox::outlineColourId, VoxiumColours::border);
		setColour(juce::ComboBox::textColourId, VoxiumColours::textPrimary);
		setColour(juce::ComboBox::arrowColourId, VoxiumColours::accent);
		setColour(juce::ComboBox::focusedOutlineColourId, VoxiumColours::accent);

		setColour(juce::PopupMenu::backgroundColourId, VoxiumColours::panel);
		setColour(juce::PopupMenu::textColourId, VoxiumColours::textPrimary);
		setColour(juce::PopupMenu::highlightedBackgroundColourId, VoxiumColours::accent);
		setColour(juce::PopupMenu::highlightedTextColourId, VoxiumColours::background);
	}

	juce::Font getComboBoxFont(juce::ComboBox&) override
	{
		return juce::Font(juce::FontOptions(15.0f));
	}

	juce::Font getLabelFont(juce::Label& label) override
	{
		return label.getFont();
	}

	// Knob rotativo minimalista: trilho circular fino, arco de valor em
	// violeta, sem ponteiro/agulha nem textura -- mesmo espirito visual
	// do resto do tema (linhas finas, um unico accent)
	void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
		float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider&) override
	{
		auto bounds = juce::Rectangle<float>((float)x, (float)y, (float)width, (float)height).reduced(4.0f);
		float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
		auto centre = bounds.getCentre();
		float lineThickness = 3.0f;

		// trilho de fundo (o "range" inteiro do knob)
		juce::Path track;
		track.addCentredArc(centre.x, centre.y, radius, radius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
		g.setColour(VoxiumColours::border);
		g.strokePath(track, juce::PathStrokeType(lineThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

		// arco preenchido ate o valor atual
		float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
		juce::Path valueArc;
		valueArc.addCentredArc(centre.x, centre.y, radius, radius, 0.0f, rotaryStartAngle, angle, true);
		g.setColour(VoxiumColours::accent);
		g.strokePath(valueArc, juce::PathStrokeType(lineThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

		// marcador (linha fina) na posicao atual
		juce::Point<float> markerEnd(centre.x + (radius - 8.0f) * std::cos(angle - juce::MathConstants<float>::halfPi),
			centre.y + (radius - 8.0f) * std::sin(angle - juce::MathConstants<float>::halfPi));
		g.setColour(VoxiumColours::textPrimary);
		g.drawLine({ centre, markerEnd }, 2.0f);
	}

	// Caixa retangular arredondada, fina, com destaque violeta quando aberta --
	// substitui o visual "padrao Windows" default do JUCE
	void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
		int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box) override
	{
		juce::ignoreUnused(isButtonDown, buttonX, buttonY, buttonW, buttonH);

		auto bounds = juce::Rectangle<float>(0, 0, (float)width, (float)height).reduced(0.5f);
		float cornerSize = 6.0f;

		g.setColour(VoxiumColours::panel);
		g.fillRoundedRectangle(bounds, cornerSize);

		bool isOpenOrFocused = box.isPopupActive() || box.hasKeyboardFocus(true);
		g.setColour(isOpenOrFocused ? VoxiumColours::accent : VoxiumColours::border);
		g.drawRoundedRectangle(bounds, cornerSize, 1.0f);

		// seta customizada, minimalista (triangulo simples), na cor accent
		float arrowSize = 5.0f;
		float arrowCenterX = (float)width - 18.0f;
		float arrowCenterY = (float)height * 0.5f;

		juce::Path arrow;
		arrow.addTriangle(arrowCenterX - arrowSize, arrowCenterY - arrowSize * 0.5f,
			arrowCenterX + arrowSize, arrowCenterY - arrowSize * 0.5f,
			arrowCenterX, arrowCenterY + arrowSize * 0.6f);

		g.setColour(VoxiumColours::accent);
		g.fillPath(arrow);
	}

	void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override
	{
		label.setBounds(1, 1, box.getWidth() - 30, box.getHeight() - 2);
		label.setFont(getComboBoxFont(box));
		label.setJustificationType(juce::Justification::centredLeft);
	}
};