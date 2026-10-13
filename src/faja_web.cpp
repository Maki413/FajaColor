#include "faja_hw.h"

namespace faja {

namespace {
const char *rgbColorName(DetectedColor color) {
  return color == DetectedColor::UNKNOWN ? "Apagado" : detectedColorName(color);
}

const char *rgbColorHex(DetectedColor color) {
  switch (color) {
    case DetectedColor::BLUE:
      return "#3787f5";
    case DetectedColor::GREEN:
      return "#20b981";
    case DetectedColor::RED:
      return "#ef5350";
    default:
      return "#aab4c3";
  }
}

const char *motorStateName(bool motorEnabled) {
  return motorEnabled ? "Encendido" : "Apagado";
}

const char *processStateName(SystemState state) {
  switch (state) {
    case SystemState::RUNNING:
      return "En marcha";
    case SystemState::ANALYZING:
      return "Clasificando objeto";
    case SystemState::WAITING_OUTPUT:
      return "Salida de objeto";
    default:
      return "Detenida";
  }
}

bool motorIsEnabled(const WebStatus& status) {
  return status.systemState == SystemState::RUNNING ||
         status.systemState == SystemState::WAITING_OUTPUT;
}

}  // namespace

String buildWebPage(const WebStatus& status) {
  const bool motorEnabled = motorIsEnabled(status);
  String page;
  page.reserve(7000);
  page = R"HTML(
<!doctype html>
<html lang="es">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta name="theme-color" content="#101c2d">
  <title>Faja de Color | Panel de control</title>
  <style>
    :root { color-scheme: light; font-family: Inter, "Segoe UI", Arial, sans-serif; color: #172438; background: #f2f5f9; }
    * { box-sizing: border-box; }
    body { margin: 0; min-width: 320px; }
    header { background: #101c2d; color: #fff; }
    .header-inner, main { width: min(1080px, calc(100% - 40px)); margin: 0 auto; }
    .header-inner { min-height: 92px; display: flex; align-items: center; justify-content: space-between; gap: 20px; }
    .brand { display: flex; align-items: center; gap: 14px; }
    .brand-mark { width: 44px; height: 44px; display: grid; place-items: center; border-radius: 12px; background: #1c8b75; color: #fff; font-size: 14px; font-weight: 800; letter-spacing: .04em; }
    h1 { margin: 0; font-size: 20px; letter-spacing: -.02em; }
    .subtitle { margin-top: 5px; color: #aebbd0; font-size: 13px; }
    .connection { display: inline-flex; align-items: center; gap: 8px; color: #b9c7d8; font-size: 12px; }
    .connection-dot { width: 8px; height: 8px; border-radius: 50%; background: #39c890; box-shadow: 0 0 0 4px rgba(57,200,144,.13); }
    .connection.offline { color: #ffb4ad; }
    .connection.offline .connection-dot { background: #f07467; box-shadow: 0 0 0 4px rgba(240,116,103,.13); }
    main { padding: 34px 0 48px; }
    .section-heading { display: flex; align-items: end; justify-content: space-between; margin: 0 0 16px; }
    .section-heading h2 { margin: 0; font-size: 16px; font-weight: 700; }
    .section-heading p { margin: 0; color: #78869a; font-size: 12px; }
    .metrics { display: grid; grid-template-columns: repeat(4, minmax(0, 1fr)); gap: 16px; }
    .card { min-height: 148px; padding: 21px; border: 1px solid #e4eaf1; border-radius: 14px; background: #fff; box-shadow: 0 4px 14px rgba(23,36,56,.035); }
    .total-card { grid-column: span 1; background: #173b56; border-color: #173b56; color: #fff; }
    .label { color: #738197; font-size: 11px; font-weight: 700; letter-spacing: .075em; text-transform: uppercase; }
    .total-card .label { color: #b8d1e0; }
    .count { margin-top: 17px; color: #172438; font-size: 38px; font-weight: 750; line-height: 1; letter-spacing: -.045em; font-variant-numeric: tabular-nums; }
    .total-card .count { color: #fff; font-size: 48px; }
    .metric-foot { margin-top: 12px; color: #8290a3; font-size: 12px; }
    .total-card .metric-foot { color: #b8d1e0; }
    .color-label { display: flex; align-items: center; gap: 8px; }
    .swatch { width: 9px; height: 9px; border-radius: 50%; }
    .blue { background: #3787f5; } .green { background: #20b981; } .red { background: #ef5350; }
    .details { display: grid; grid-template-columns: repeat(3, minmax(0, 1fr)); gap: 16px; margin-top: 22px; }
    .detail-card { min-height: 164px; }
    .detail-main { display: flex; align-items: center; gap: 14px; margin-top: 20px; }
    .detail-value { color: #172438; font-size: 20px; font-weight: 700; }
    .detail-note { margin-top: 6px; color: #8290a3; font-size: 12px; }
    .led-orb { width: 42px; height: 42px; flex: 0 0 auto; border: 6px solid #f1f4f7; border-radius: 50%; background: #aab4c3; box-shadow: 0 0 0 1px #e5eaf0; }
    .motor-icon { width: 42px; height: 42px; display: grid; place-items: center; flex: 0 0 auto; border-radius: 12px; background: #edf2f6; color: #748399; font-size: 20px; }
    .motor-icon.on { background: #e5f7ef; color: #149462; }
    .state-pill { display: inline-flex; align-items: center; gap: 7px; margin-top: 16px; padding: 7px 10px; border-radius: 20px; background: #f0f3f7; color: #66758a; font-size: 11px; font-weight: 650; }
    .state-pill.on { background: #e5f7ef; color: #148158; }
    .state-pill::before { content: ""; width: 6px; height: 6px; border-radius: 50%; background: currentColor; }
    .footer { display: flex; justify-content: space-between; gap: 12px; margin-top: 20px; color: #8793a4; font-size: 11px; }
    @media (max-width: 760px) { .metrics { grid-template-columns: repeat(2, minmax(0, 1fr)); } .details { grid-template-columns: 1fr; } .detail-card { min-height: 140px; } }
    @media (max-width: 480px) { .header-inner, main { width: min(100% - 28px, 1080px); } .header-inner { min-height: 82px; } .brand-mark { width: 40px; height: 40px; } h1 { font-size: 17px; } .subtitle { font-size: 11px; } .connection { font-size: 0; } .metrics { gap: 10px; } .card { min-height: 132px; padding: 16px; } .count { font-size: 32px; } .total-card .count { font-size: 40px; } .label { font-size: 10px; } .footer { flex-direction: column; } }
  </style>
</head>
<body>
  <header>
    <div class="header-inner">
      <div class="brand">
        <div class="brand-mark">FC</div>
        <div><h1>Faja de Color</h1><div class="subtitle">Panel de monitoreo de clasificación</div></div>
      </div>
      <div class="connection" id="connectionBadge"><span class="connection-dot"></span><span id="connectionText">Sistema conectado</span></div>
    </div>
  </header>
  <main>
    <section aria-labelledby="production-title">
      <div class="section-heading"><h2 id="production-title">Producción</h2><p>Conteo acumulado desde el encendido</p></div>
      <div class="metrics">
        <article class="card total-card">
          <div class="label">Objetos procesados</div>
          <div class="count" id="objectCount">)HTML";
  page += String(status.objectCount);
  page += R"HTML(</div>
          <div class="metric-foot">Total de objetos clasificados</div>
        </article>
        <article class="card">
          <div class="label color-label"><span class="swatch blue"></span>Azul</div>
          <div class="count" id="blueCount">)HTML";
  page += String(status.blueCount);
  page += R"HTML(</div><div class="metric-foot">Objetos clasificados</div>
        </article>
        <article class="card">
          <div class="label color-label"><span class="swatch green"></span>Verde</div>
          <div class="count" id="greenCount">)HTML";
  page += String(status.greenCount);
  page += R"HTML(</div><div class="metric-foot">Objetos clasificados</div>
        </article>
        <article class="card">
          <div class="label color-label"><span class="swatch red"></span>Rojo</div>
          <div class="count" id="redCount">)HTML";
  page += String(status.redCount);
  page += R"HTML(</div><div class="metric-foot">Objetos clasificados</div>
        </article>
      </div>
    </section>
    <section aria-labelledby="system-title" style="margin-top:30px">
      <div class="section-heading"><h2 id="system-title">Estado del sistema</h2><p>Actualización automática en tiempo real</p></div>
      <div class="details">
        <article class="card detail-card">
          <div class="label">Último objeto detectado</div>
          <div class="detail-main"><span class="swatch" id="detectedSwatch" style="width:14px;height:14px;background:)HTML";
  page += rgbColorHex(status.latestColor);
  page += R"HTML("></span><div><div class="detail-value" id="latestColor">)HTML";
  page += detectedColorName(status.latestColor);
  page += R"HTML(</div><div class="detail-note">Color identificado por el sensor</div></div></div>
        </article>
        <article class="card detail-card">
          <div class="label">LED RGB</div>
          <div class="detail-main"><span class="led-orb" id="rgbLed" style="background:)HTML";
  page += rgbColorHex(status.rgbColor);
  page += R"HTML("></span><div><div class="detail-value" id="rgbColor">)HTML";
  page += rgbColorName(status.rgbColor);
  page += R"HTML(</div><div class="detail-note">Color indicado durante la clasificación</div></div></div>
        </article>
        <article class="card detail-card">
          <div class="label">Motor de la faja</div>
          <div class="detail-main"><span class="motor-icon)HTML";
  page += motorEnabled ? " on" : "";
  page += R"HTML(" id="motorIcon">&#9881;</span><div><div class="detail-value" id="motorStatus">)HTML";
  page += motorStateName(motorEnabled);
  page += R"HTML(</div><div class="detail-note" id="motorNote">)HTML";
  page += processStateName(status.systemState);
  page += R"HTML(</div></div></div>
          <div class="state-pill)HTML";
  page += motorEnabled ? " on" : "";
  page += R"HTML(" id="processState">)HTML";
  page += processStateName(status.systemState);
  page += R"HTML(</div>
        </article>
      </div>
    </section>
    <div class="footer"><span>Faja de Color · Monitoreo local</span><span>Los datos se actualizan automáticamente</span></div>
  </main>
  <script>
    const colorStyles = {
      "Azul": "#3787f5",
      "Verde": "#20b981",
      "Rojo": "#ef5350",
      "Apagado": "#aab4c3",
      "Sin color": "#aab4c3"
    };
    const processStates = ["Detenida", "En marcha", "Clasificando objeto", "Salida de objeto"];
    function setText(id, value) {
      document.getElementById(id).textContent = value;
    }
    async function refresh() {
      const badge = document.getElementById('connectionBadge');
      try {
        const response = await fetch('/status', { cache: 'no-store' });
        if (!response.ok) throw new Error('No se pudo leer el estado');
        const data = await response.json();
        setText('objectCount', data.objectCount);
        setText('blueCount', data.blueCount);
        setText('greenCount', data.greenCount);
        setText('redCount', data.redCount);
        setText('latestColor', data.latestColor);
        const detectedSwatch = document.getElementById('detectedSwatch');
        detectedSwatch.style.backgroundColor = colorStyles[data.latestColor] || colorStyles['Sin color'];
        setText('rgbColor', data.rgbColor);
        document.getElementById('rgbLed').style.backgroundColor = colorStyles[data.rgbColor] || colorStyles['Apagado'];
        setText('motorStatus', data.motor);
        const running = data.motor === 'Encendido';
        document.getElementById('motorIcon').classList.toggle('on', running);
        const state = processStates[data.systemState] || processStates[0];
        setText('motorNote', state);
        setText('processState', state);
        document.getElementById('processState').classList.toggle('on', running);
        badge.classList.remove('offline');
        setText('connectionText', 'Sistema conectado');
      } catch (error) {
        badge.classList.add('offline');
        setText('connectionText', 'Sin conexión');
      }
    }
    refresh();
    setInterval(refresh, 750);
  </script>
</body>
</html>
)HTML";
  return page;
}

String buildStatusJson(const WebStatus& status) {
  return String("{\"objectCount\":") + String(status.objectCount) +
         ",\"blueCount\":" + String(status.blueCount) +
         ",\"greenCount\":" + String(status.greenCount) +
         ",\"redCount\":" + String(status.redCount) +
         ",\"latestColor\":\"" + detectedColorName(status.latestColor) +
         "\",\"rgbColor\":\"" + rgbColorName(status.rgbColor) +
         "\",\"motor\":\"" + motorStateName(motorIsEnabled(status)) +
         "\",\"systemState\":" + String(static_cast<uint8_t>(status.systemState)) + "}";
}

}  // namespace faja
