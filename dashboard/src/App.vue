<template>
  <div
    class="container-md vertical-center overflow-hidden"
    v-on:click="showTilt = !showTilt"
  >
    <div v-if="error">
      <h1>Error reading the data file</h1>
    </div>
    <div v-else-if="!showTilt" class="col-12">
      <div class="row badges mb-4">
        <badge
          v-if="typeof engine.vehicleSpeed != 'undefined'"
          name="Speed"
          unit="km/h"
          :value="engine.vehicleSpeed"
        ></badge>

        <badge
          v-if="typeof engine.engineRPM != 'undefined'"
          name="RPM"
          :value="engine.engineRPM"
        ></badge>

        <badge
          v-if="typeof engine.fuelLevel != 'undefined'"
          name="Fuel level"
          unit="%"
          :value="engine.fuelLevel"
        ></badge>

        <div class="clearfix"></div>

        <badge
          v-if="typeof engine.ambientAirTemperature != 'undefined'"
          name="Air temperature"
          unit="C°"
          :value="engine.ambientAirTemperature"
        ></badge>

        <badge
          v-if="typeof engine.intakeAirTemperature != 'undefined'"
          name="Air intake temperature"
          unit="C°"
          :value="engine.intakeAirTemperature"
        ></badge>

        <badge
          v-if="typeof engine.oilTemperature != 'undefined'"
          name="Oil temperature"
          unit="C°"
          :value="engine.oilTemperature"
        ></badge>

        <badge
          v-if="typeof engine.coolantTemperature != 'undefined'"
          name="Coolant temperature"
          unit="C°"
          :value="engine.coolantTemperature"
        ></badge>
      </div>

      <div class="row progress-bars">
        <progress-bar
          v-if="typeof engine.engineLoad != 'undefined'"
          name="Engine load"
          unit="%"
          :level="engine.engineLoad"
          :min="0"
          :max="100"
        ></progress-bar>

        <progress-bar
          v-if="typeof engine.relativeThrottlePosition != 'undefined'"
          name="Throttle position"
          unit="%"
          :level="engine.relativeThrottlePosition"
          :min="0"
          :max="100"
          is_graded="true"
        ></progress-bar>

        <progress-bar
          v-if="typeof engine.actualTorque != 'undefined'"
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
      <tilt
        v-if="typeof engine.xTilt != 'undefined'"
        name="Slope"
        :value="engine.xTilt"
      ></tilt>

      <tilt
        v-if="typeof engine.yTilt != 'undefined'"
        name="Swinging"
        :value="engine.yTilt"
      ></tilt>
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
      error: false,
      showTilt: false,
      engine: {
        // Badges (I)
        vehicleSpeed: 95, // VEHICLE_SPEED
        engineRPM: 500, // ENGINE_RPM
        fuelLevel: 45, // FUEL_TANK_LEVEL_INPUT

        // Badges (II)
        ambientAirTemperature: 32, // AMBIENT_AIR_TEMP
        oilTemperature: 45, // ENGINE_OIL_TEMP
        coolantTemperature: 45, // ENGINE_COOLANT_TEMP
        intakeAirTemperature: 60, // INTAKE_AIR_TEMP

        // Progress bars
        engineLoad: 45, // ENGINE_LOAD
        relativeThrottlePosition: 71, // RELATIVE_THROTTLE_POSITION
        actualTorque: 91, // ACTUAL_ENGINE_TORQUE

        xTilt: 30, // XTILT
        yTilt: 60, // YTILT
      },
    };
  },
  mounted() {
    if (window.EventSource) {
      var source = new EventSource("/events");

      source.onopen = () => console.log("Events source connected");
      source.onerror = (e) => {
        if (e.target.readyState != EventSource.OPEN) {
          console.log("Event source closed");
          // this.error = true;
          // Hopefully the board will try reconnecting...
        }
      };

      source.addEventListener(
        "dataupdate",
        (e) => this.updateEngine(e.data),
        false
      );
    }

    // Random data
    setInterval(() => {
      for (let key in this.engine)
        this.engine[key] += Math.round(
          this.engine[key] * (Math.random() - 0.5) * 0.1
        );
    }, 1000);
  },
  methods: {
    updateEngine(data) {
      var value = JSON.parse(data);
      if (value["error"] != "undefined") {
        this.error = true;
        console.log("OBD error, check board log");
      } else {
        this.engine = JSON.parse(data);
      }
    },
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

.badges > div {
  min-width: 25%;
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
