import { describe, it, expect } from "vitest";
import { PARAMS, formatValue } from "../params";

describe("param table", () => {
  it("has 13 unique ids matching the C++ spec count", () => {
    expect(PARAMS.length).toBe(13);
    expect(new Set(PARAMS.map((p) => p.id)).size).toBe(13);
  });
});

describe("formatValue mirrors vial display scaling", () => {
  it("linear percentage", () => {
    expect(formatValue("reverb_dry_wet", 0.25)).toBe("25%");
  });

  it("exponential seconds (decay = 2^x)", () => {
    expect(formatValue("reverb_decay_time", 0)).toBe("1 secs"); // 2^0 = 1 s
    expect(formatValue("reverb_decay_time", 1)).toBe("2 secs"); // 2^1 = 2 s
  });

  it("quadratic percentage (x^2 * 100)", () => {
    expect(formatValue("reverb_chorus_amount", 0.5)).toBe("25%"); // 0.5^2 * 100
  });
});
