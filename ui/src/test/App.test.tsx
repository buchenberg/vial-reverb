import { describe, it, expect } from "vitest";
import { render, screen, fireEvent } from "@testing-library/react";
import App from "../App";
import { Knob } from "../components/Knob";
import { getSliderState } from "../juce/index.js";

describe("App layout", () => {
  it("renders the reverb panel", () => {
    render(<App />);
    expect(document.querySelector('[data-panel="reverb"]')).toBeInTheDocument();
  });

  it("renders the expected control inventory", () => {
    render(<App />);
    expect(document.querySelector('[data-param="reverb_decay_time"]')).toBeInTheDocument();
    expect(document.querySelector('[data-param="reverb_size"]')).toBeInTheDocument();
    expect(document.querySelector('[data-param="reverb_dry_wet"]')).toBeInTheDocument();
    expect(document.querySelector('[data-param="reverb_stereo_width"]')).toBeInTheDocument();
  });
});

describe("control bindings", () => {
  it("scrolling a knob updates the bound slider value", () => {
    render(<Knob id="reverb_size" label="Size" />);
    const dial = screen.getByRole("slider", { name: "Size" });
    const before = getSliderState("reverb_size").getScaledValue();
    fireEvent.wheel(dial, { deltaY: -100 }); // scroll up -> increase
    expect(getSliderState("reverb_size").getScaledValue()).toBeGreaterThan(before);
  });
});
