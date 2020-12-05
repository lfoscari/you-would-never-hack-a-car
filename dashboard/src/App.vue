<template>
  <div
    class="container-md vertical-center overflow-hidden"
    v-on:click="showTilt = !showTilt"
  >
    <div v-if="error != ''">
      <h1>{{ error }}</h1>
    </div>
    <div v-else-if="!showTilt" class="col-12">
      <div class="row mb-4">
        <badge name="Speed" unit="km/h" :value="engine.vehicleSpeed"></badge>
        <badge name="RPM" :value="engine.engineRPM"></badge>
        <badge name="Fuel level" unit="%" :value="engine.fuelLevel"></badge>

        <div class="clearfix"></div>

        <badge
          name="Air temperature"
          unit="C°"
          :value="Mathengine.ambientAirTemperature"
        ></badge>

        <badge
          name="Air intake temperature"
          unit="C°"
          :value="engine.intakeAirTemperature"
        ></badge>

        <badge
          name="Oil temperature"
          unit="C°"
          :value="engine.oilTemperature"
        ></badge>

        <badge
          name="Coolant temperature"
          unit="C°"
          :value="engine.coolantTemperature"
        ></badge>
      </div>

      <div class="row">
        <progress-bar
          name="Engine load"
          unit="%"
          :level="engine.engineLoad"
          :min="0"
          :max="100"
        ></progress-bar>

        <progress-bar
          name="Throttle position"
          unit="%"
          :level="engine.relativeThrottlePosition"
          :min="0"
          :max="100"
          is_graded="true"
        ></progress-bar>

        <progress-bar
          name="Actual torque"
          unit="%"
          :level="engine.actualTorque"
          :min="0"
          :max="100"
          is_graded="true"
        ></progress-bar>
      </div>
    </div>
    <div v-else class="row">
      <tilt name="Pitching" :value="engine.yTilt"></tilt>
      <tilt name="Rolling" :value="engine.zTilt"></tilt>
    </div>
  </div>
</template>

<script>
import ProgressBar from "./components/ProgressBar.vue";
import Badge from "./components/Badge.vue";
import Tilt from "./components/Tilt.vue";

export default {
  name: "App",
  components: {
    ProgressBar,
    Badge,
    Tilt,
  },

  data() {
    return {
      error: "",
      showTilt: false,
      engine: {
        // Badges (I)
        vehicleSpeed: undefined, // VEHICLE_SPEED
        engineRPM: undefined, // ENGINE_RPM
        fuelLevel: undefined, // FUEL_TANK_LEVEL_INPUT

        // Badges (II)
        ambientAirTemperature: undefined, // AMBIENT_AIR_TEMP
        oilTemperature: undefined, // ENGINE_OIL_TEMP
        coolantTemperature: undefined, // ENGINE_COOLANT_TEMP
        intakeAirTemperature: undefined, // INTAKE_AIR_TEMP

        // Progress bars
        engineLoad: undefined, // ENGINE_LOAD
        relativeThrottlePosition: undefined, // RELATIVE_THROTTLE_POSITION
        actualTorque: undefined, // ACTUAL_ENGINE_TORQUE

        yTilt: undefined, // YTILT
        zTilt: undefined, // ZTILT
      },
    };
  },
  mounted() {
    if (window.EventSource) {
      var source = new EventSource("/events");

      source.onopen = () => console.log("Events source connected");

      Object.keys(this.engine).forEach((key) => {
        source.addEventListener(key, (m) => (this.engine[key] = m.data));
      });

      source.addEventListener("error", (e) => {
        if (e.target.readyState != EventSource.OPEN)
          console.log("Event source closed");
        console.log("ERROR: ", e.data);

        this.error = "There's been an error, reloading...";
        setTimeout(() => (location = location.href), 2000);
      });
    }
  },
};
</script>

<style>
body,
html {
  height: 100%;
}

body {
  background-color: var(--bs-dark);
  color: var(--bs-light);
}

.row {
  align-items: center;
}

.progress {
  background-color: var(--bs-gray);
  background: repeating-linear-gradient(
    90deg,
    var(--bs-gray),
    var(--bs-gray) 5px,
    var(--bs-gray-dark) 5px,
    var(--bs-gray-dark) 10px
  );
}

.vertical-center {
  min-height: 100%; /* fallback */
  min-height: 100vh;

  display: flex;
  align-items: center;
}
</style>
