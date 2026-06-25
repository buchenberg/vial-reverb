import Box from "@mui/material/Box";
import { Panel } from "./components/Panel";
import { SubPanel } from "./components/SubPanel";
import { Knob } from "./components/Knob";
import { ReverbEq } from "./components/Visualizers";

export default function App() {
  return (
    <Box sx={{ display: "flex", flexDirection: "column", height: "100vh", p: 1, gap: 1 }}>
      <Panel title="VIAL REVERB" kind="reverb">
        <Box sx={{ display: "flex", flex: 1, gap: 1, width: "100%", minHeight: 0, alignItems: "stretch" }}>
          {/* Prefilters */}
          <SubPanel title="Prefilters">
            <Box sx={{ display: "flex", flexDirection: "column", gap: 1 }}>
              <Knob id="reverb_pre_low_cutoff" label="Low Cutoff" />
              <Knob id="reverb_pre_high_cutoff" label="High Cutoff" />
            </Box>
          </SubPanel>

          {/* EQ / Tone */}
          <SubPanel title="Tone" sx={{ flex: 1, minWidth: 0 }}>
            <Box sx={{ display: "flex", flexDirection: "column", flex: 1, gap: 1, width: "100%" }}>
              <Box sx={{ flex: 1, minHeight: 0 }}>
                <ReverbEq
                  lowCutId="reverb_low_shelf_cutoff"
                  lowGainId="reverb_low_shelf_gain"
                  highCutId="reverb_high_shelf_cutoff"
                  highGainId="reverb_high_shelf_gain"
                />
              </Box>
              <Box sx={{ display: "grid", gridTemplateColumns: "1fr 1fr", gap: 1 }}>
                <Box sx={{ display: "flex", flexDirection: "column", gap: 0.5, alignItems: "center" }}>
                  <Knob id="reverb_low_shelf_cutoff" label="Low Freq" />
                  <Knob id="reverb_low_shelf_gain" label="Low Gain" />
                </Box>
                <Box sx={{ display: "flex", flexDirection: "column", gap: 0.5, alignItems: "center" }}>
                  <Knob id="reverb_high_shelf_cutoff" label="High Freq" />
                  <Knob id="reverb_high_shelf_gain" label="High Gain" />
                </Box>
              </Box>
            </Box>
          </SubPanel>

          {/* Reverb core */}
          <SubPanel title="Reverb">
            <Box sx={{ display: "grid", gridTemplateColumns: "1fr 1fr", gap: 1 }}>
              <Knob id="reverb_dry_wet" label="Mix" />
              <Knob id="reverb_decay_time" label="Decay" />
              <Knob id="reverb_delay" label="Delay" />
              <Knob id="reverb_size" label="Size" />
            </Box>
          </SubPanel>

          {/* Chorus */}
          <SubPanel title="Chorus">
            <Box sx={{ display: "flex", flexDirection: "column", gap: 1 }}>
              <Knob id="reverb_chorus_amount" label="Amount" />
              <Knob id="reverb_chorus_frequency" label="Rate" />
            </Box>
          </SubPanel>

          {/* Output */}
          <SubPanel title="Output">
            <Knob id="reverb_stereo_width" label="Width" />
          </SubPanel>
        </Box>
      </Panel>
    </Box>
  );
}
