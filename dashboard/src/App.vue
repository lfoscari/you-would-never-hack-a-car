<template>
  <div
    class="container-md vertical-center overflow-hidden"
    v-on:click="showTilt = !showTilt"
  >
    <div v-if="message != ''">
      <h1>{{ message }}</h1>
    </div>
    <div v-else-if="!showTilt" class="col-12">
      <div class="row mb-4">

        <badge
          name="Speed"
          unit="km/h"
          :value="engine.vehicleSpeed"
        ></badge>

        <badge 
          name="RPM"
          :value="engine.engineRPM"
        ></badge>

        <badge
          name="Intake pressure"
          unit="kPa"
          :value="engine.intakePressure"
        ></badge>

        <!-- <badge name="Fuel level" unit="%" :value="engine.fuelLevel"></badge> -->

        <div class="clearfix"></div>

        <badge
          name="Air intake temperature"
          unit="C°"
          :value="engine.intakeAirTemperature"
        ></badge>

        <!-- <badge
          name="Oil temperature"
          unit="C°"
          :value="engine.oilTemperature"
        ></badge> -->

        <badge 
          name="Air temperature"
          unit="C°"
          :value="engine.ambientAirTemperature"
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

        <!-- <progress-bar
          name="Actual torque"
          unit="%"
          :level="engine.actualTorque"
          :min="0"
          :max="100"
          is_graded="true"
        ></progress-bar> -->

      </div>
    </div>
    <div v-else class="row">

      <tilt
        name="Pitch"
        :value="engine.yTilt"
      ></tilt>
      
      <tilt
        name="Roll"
        :value="engine.xTilt"
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
      message: "",
      showTilt: false,
      engine: {
        // Badges (I)
        vehicleSpeed: undefined, // VEHICLE_SPEED
        engineRPM: undefined, // ENGINE_RPM
        intakePressure: undefined, // INTAKE_MANIFOLD_ABS_PRESSURE

        // Badges (II)
        intakeAirTemperature: undefined, // INTAKE_AIR_TEMP
        ambientAirTemperature: undefined, // AMBIENT_AIR_TEMP
        coolantTemperature: undefined, // ENGINE_COOLANT_TEMP

        // Progress bars
        engineLoad: undefined, // ENGINE_LOAD
        relativeThrottlePosition: undefined, // RELATIVE_THROTTLE_POSITION

        xTilt: undefined, // XTILT
        yTilt: undefined, // YTILT
      },
    };
  },
  mounted() {
    // setInterval(() => Object.keys(this.engine).forEach((key) => this.engine[key] = Math.floor(Math.random() * 100)), 1000);

    if (window.WebSocket) {
      var socket = new WebSocket(`ws://${window.location.host}/engine`);
      this.message = "Loading...";

      socket.onmessage = (event) => {
        var data = event.data.split(":");
        if(data[0] == "error") {
          this.message = data[1];
        } else {
          this.engine[data[0]] = data[1];
          this.message = "";
        }
      };
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
