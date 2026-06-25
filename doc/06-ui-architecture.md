# UI Architecture

## Overview

The vial-reverb plugin uses a **WebView-based UI** built with modern web technologies. This approach provides:

- Rapid UI development with hot reload
- Access to modern UI frameworks (React, MUI)
- Platform-native look and feel
- Easy iteration on design

## Technology Stack

### Frontend Framework
- **React 18**: Component-based UI library
- **TypeScript**: Type-safe JavaScript
- **Vite**: Fast build tool and dev server
- **MUI (Material-UI)**: Component library with Material Design

### Build Tools
- **npm**: Package manager
- **Vite**: Bundler and dev server
- **TypeScript Compiler**: Type checking

### JUCE Integration
- **juce::WebViewComponent**: Hosts the web content
- **JavaScript Interop**: Bidirectional communication between C++ and JavaScript
- **Binary Data**: UI assets embedded in plugin binary

## Project Structure

```
ui/
├── src/
│   ├── components/          # React components
│   │   ├── Knob.tsx         # Rotary knob control
│   │   ├── Panel.tsx        # Container panel
│   │   ├── ParamSlider.tsx  # Slider for parameters
│   │   ├── SubPanel.tsx     # Sub-section panel
│   │   └── Visualizers.tsx  # Audio visualizations
│   ├── juce/                # JUCE interop layer
│   │   ├── index.js         # JavaScript bindings
│   │   ├── index.d.ts       # TypeScript definitions
│   │   ├── mock.ts          # Mock for development
│   │   └── check_native_interop.js
│   ├── hooks.ts             # Custom React hooks
│   ├── params.ts            # Parameter definitions
│   ├── theme.ts             # MUI theme configuration
│   ├── styles.css           # Global styles
│   ├── App.tsx              # Root component
│   └── main.tsx             # Entry point
├── test/                    # Test files
│   ├── App.test.tsx
│   ├── params.test.ts
│   └── setup.ts
├── index.html               # HTML template
├── package.json             # Dependencies
├── tsconfig.json            # TypeScript config
├── vite.config.ts           # Vite config
└── vitest.config.ts         # Test config
```

## Component Architecture

### Root Component (App.tsx)

The `App` component is the root of the UI tree:

```tsx
function App() {
  const parameters = useParameters();  // Hook to get parameter values
  
  return (
    <ThemeProvider theme={theme}>
      <Panel>
        <Knob param={parameters.dryWet} label="Dry/Wet" />
        <Knob param={parameters.decayTime} label="Decay" />
        {/* ... more knobs */}
      </Panel>
    </ThemeProvider>
  );
}
```

### Parameter Components

#### Knob.tsx

Rotary knob control for continuous parameters:

```tsx
interface KnobProps {
  param: Parameter;
  label: string;
}

function Knob({ param, label }: KnobProps) {
  const [value, setValue] = useState(param.value);
  
  const handleChange = (newValue: number) => {
    setValue(newValue);
    juce.sendParameterChange(param.id, newValue);
  };
  
  return (
    <div className="knob">
      <RotaryControl value={value} onChange={handleChange} />
      <span className="label">{label}</span>
    </div>
  );
}
```

#### ParamSlider.tsx

Linear slider for parameters:

```tsx
function ParamSlider({ param, label }: ParamSliderProps) {
  return (
    <div className="param-slider">
      <label>{label}</label>
      <Slider
        value={param.value}
        min={param.min}
        max={param.max}
        onChange={(e, value) => juce.sendParameterChange(param.id, value)}
      />
    </div>
  );
}
```

### Container Components

#### Panel.tsx

Container with styling:

```tsx
function Panel({ children }: PanelProps) {
  return (
    <div className="panel">
      {children}
    </div>
  );
}
```

## JUCE Interop

### JavaScript Bindings (juce/index.js)

The `juce` object provides bidirectional communication:

```javascript
// Send parameter change to C++
window.juce.sendParameterChange = (paramId, value) => {
  if (window.__juce_ready) {
    window.__juce_call('setParameter', [paramId, value]);
  }
};

// Receive parameter updates from C++
window.juce.onParameterChange = (callback) => {
  window.__juce_onParameterChange = callback;
};
```

### TypeScript Definitions (juce/index.d.ts)

Type-safe interfaces for JUCE interop:

```typescript
declare namespace juce {
  function sendParameterChange(paramId: string, value: number): void;
  function onParameterChange(callback: (paramId: string, value: number) => void): void;
  function getParameter(paramId: string): Promise<number>;
}
```

### Mock for Development (juce/mock.ts)

Mock implementation for development without the plugin:

```typescript
export const mockJuce = {
  sendParameterChange: (paramId: string, value: number) => {
    console.log(`[Mock] Parameter ${paramId} = ${value}`);
  },
  onParameterChange: (callback: Function) => {
    // No-op in mock
  },
  getParameter: async (paramId: string) => {
    return 0.5; // Default value
  }
};
```

## Parameter Management

### Parameter Definitions (params.ts)

Central parameter definitions:

```typescript
export interface Parameter {
  id: string;
  name: string;
  min: number;
  max: number;
  defaultValue: number;
  scaling: 'linear' | 'exponential' | 'quadratic';
}

export const parameters: Parameter[] = [
  {
    id: 'reverb_dry_wet',
    name: 'Dry/Wet',
    min: 0.0,
    max: 1.0,
    defaultValue: 0.25,
    scaling: 'linear'
  },
  // ... more parameters
];
```

### Custom Hooks (hooks.ts)

React hooks for parameter management:

```typescript
export function useParameters() {
  const [values, setValues] = useState<Record<string, number>>({});
  
  useEffect(() => {
    // Initialize with default values
    const defaults = parameters.reduce((acc, p) => {
      acc[p.id] = p.defaultValue;
      return acc;
    }, {} as Record<string, number>);
    setValues(defaults);
    
    // Listen for parameter changes from plugin
    juce.onParameterChange((paramId, value) => {
      setValues(prev => ({ ...prev, [paramId]: value }));
    });
  }, []);
  
  return values;
}
```

## Styling

### Theme Configuration (theme.ts)

MUI theme with custom colors and typography:

```typescript
import { createTheme } from '@mui/material/styles';

export const theme = createTheme({
  palette: {
    mode: 'dark',
    primary: {
      main: '#ff9800',  // Orange accent
    },
    background: {
      default: '#1e1e1e',
      paper: '#2d2d2d',
    },
  },
  typography: {
    fontFamily: '"Inter", "Roboto", "Helvetica", "Arial", sans-serif',
  },
});
```

### Global Styles (styles.css)

```css
body {
  margin: 0;
  padding: 0;
  background: #1e1e1e;
  color: #ffffff;
  font-family: 'Inter', sans-serif;
  overflow: hidden;
}

.knob {
  display: flex;
  flex-direction: column;
  align-items: center;
  margin: 10px;
}

.panel {
  background: #2d2d2d;
  border-radius: 8px;
  padding: 20px;
}
```

## Development Workflow

### Development Mode

```bash
cd ui
npm install
npm run dev
```

This starts the Vite dev server with hot reload. The UI can be viewed in a browser at `http://localhost:5173`.

### Mock Mode

During development, the UI uses mock JUCE bindings (`juce/mock.ts`) to simulate plugin communication. This allows UI development without building the C++ plugin.

### Production Build

```bash
npm run build
```

This creates optimized production assets in `ui/dist/`:
- Minified JavaScript bundles
- Optimized CSS
- Compressed assets

### Integration with CMake

The CMake build system automatically:
1. Runs `npm install` if `node_modules` doesn't exist
2. Runs `npm run build` to create production assets
3. Embeds the assets using `juce_add_binary_data()`

## Testing

### Unit Tests

```bash
npm test
```

Tests use Vitest and React Testing Library:

```typescript
// test/App.test.tsx
import { render, screen } from '@testing-library/react';
import App from '../App';

test('renders knobs for all parameters', () => {
  render(<App />);
  const knobs = screen.getAllByRole('slider');
  expect(knobs.length).toBe(13);
});
```

### Parameter Tests

```typescript
// test/params.test.ts
import { parameters } from '../params';

test('all parameters have valid ranges', () => {
  parameters.forEach(p => {
    expect(p.min).toBeLessThan(p.max);
    expect(p.defaultValue).toBeGreaterThanOrEqual(p.min);
    expect(p.defaultValue).toBeLessThanOrEqual(p.max);
  });
});
```

## Performance Considerations

### Bundle Size

The production build is optimized for minimal bundle size:
- Tree shaking removes unused code
- Code splitting for lazy loading
- Asset compression (gzip/brotli)

### Rendering Performance

- React.memo for expensive components
- Virtual DOM diffing minimizes DOM updates
- CSS animations instead of JavaScript animations

### Memory Management

- Event listeners cleaned up in useEffect
- No memory leaks from subscriptions
- Garbage collection friendly patterns

## Future Enhancements

### Potential Improvements

1. **Preset Browser**
   - Save/load parameter presets
   - Import/export preset files
   - Preset categories and search

2. **Visualizations**
   - Real-time frequency spectrum
   - Reverb tail decay visualization
   - 3D room visualization

3. **Advanced Controls**
   - MIDI learn UI
   - Parameter automation curves
   - Macro controls

4. **Responsive Design**
   - Adaptive layout for different window sizes
   - Touch-friendly controls for tablets
   - High-DPI support

5. **Accessibility**
   - Keyboard navigation
   - Screen reader support
   - High contrast mode

## Troubleshooting

### Common Issues

#### 1. UI Not Loading

**Problem**: WebView shows blank page.

**Solution**:
- Check browser console for JavaScript errors
- Verify UI build completed successfully
- Check CMake embedded binary data

#### 2. Parameter Changes Not Reflected

**Problem**: UI updates but plugin doesn't respond.

**Solution**:
- Verify `juce.sendParameterChange()` is called
- Check C++ parameter listener is registered
- Debug JUCE interop layer

#### 3. Hot Reload Not Working

**Problem**: UI doesn't update on code changes.

**Solution**:
- Ensure dev server is running (`npm run dev`)
- Check Vite configuration
- Clear browser cache

#### 4. Build Fails

**Problem**: `npm run build` fails.

**Solution**:
- Delete `node_modules` and `package-lock.json`
- Run `npm install` again
- Check Node.js version (18+)
