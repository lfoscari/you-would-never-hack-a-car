<template>
  <div id="app" class="container-md vertical-center overflow-hidden">
    <div v-if="error">
      <h1>Error reading the data file</h1>
    </div>
    <div v-else class="col-12">
      <div class="row justify-content-center">
        <div v-if="typeof engine.vehicleSpeed != 'undefined'" class="col-sm-3 col-4 gx-2">
          <badge name="Speed" unit="km/h" :value="engine.vehicleSpeed"></badge>
        </div>

        <div v-if="typeof engine.engineRPM != 'undefined'" class="col-sm-3 col-4 gx-2">
          <badge name="RPM" :value="engine.engineRPM"></badge>
          <!-- .toString().padStart(4, '0') -->
        </div>

        <div v-if="typeof engine.fuelLevel != 'undefined'" class="col-sm-3 col-4 gx-2">
          <badge name="Fuel level" unit="%" :value="engine.fuelLevel"></badge>
        </div>
      </div>

      <div class="row justify-content-center">
        <div v-if="typeof engine.ambientAirTemperature != 'undefined'" class="col-sm-3 col-6 gx-2">
          <badge name="Air temperature" unit="C°" :value="engine.ambientAirTemperature"></badge>
        </div>

        <div v-if="typeof engine.intakeAirTemperature != 'undefined'" class="col-sm-3 col-6 gx-2">
          <badge name="Air intake temperature" unit="C°" :value="engine.intakeAirTemperature"></badge>
        </div>

        <div v-if="typeof engine.oilTemperature != 'undefined'" class="col-sm-3 col-6 gx-2">
          <badge name="Oil temperature" unit="C°" :value="engine.oilTemperature"></badge>
        </div>

        <div v-if="typeof engine.coolantTemperature != 'undefined'" class="col-sm-3 col-6 gx-2">
          <badge name="Coolant temperature" unit="C°" :value="engine.coolantTemperature"></badge>
        </div>
      </div>

      <div class="row justify-content-center">
        <div v-if="typeof engine.engineLoad != 'undefined'" class="col-sm-4 col-12 gy-3">
          <progress-bar
            name="Engine load"
            unit="%"
            :level="engine.engineLoad"
            :min="0"
            :max="100"
            is_graded="true"
          ></progress-bar>
        </div>

        <div
          v-if="typeof engine.relativeThrottlePosition != 'undefined'"
          class="col-sm-4 col-12 gy-3"
        >
          <progress-bar
            name="Throttle position"
            unit="%"
            :level="engine.relativeThrottlePosition"
            :min="0"
            :max="100"
          ></progress-bar>
        </div>

        <div v-if="typeof engine.actualTorque != 'undefined'" class="col-sm-4 col-12 gy-3">
          <progress-bar
            name="Actual torque"
            unit="%"
            :level="engine.actualTorque"
            :min="0"
            :max="100"
          ></progress-bar>
        </div>

        <!-- <div class="col-12">
          <p>Lorem ipsum dolor sit amet consectetur adipisicing elit. Fugit atque fugiat mollitia iste perferendis doloremque quos nihil ratione quasi aperiam provident a, labore inventore vel amet minus ea sapiente ab.</p>
        </div>-->
      </div>
    </div>
  </div>
</template>

<script>
import ProgressBar from "./components/ProgressBar.vue";
import Badge from "./components/Badge.vue";

export default {
  name: "App",
  components: {
    ProgressBar,
    Badge,
  },

  data() {
    return {
      error: false,
      engine: {
        // Badges (I)
        vehicleSpeed: 95, // VEHICLE_SPEED
        engineRPM: 500, // ENGINE_RPM
        fuelLevel: 45, // FUEL_TANK_LEVEL_INPUT
        // ?,

        // Badges (II)
        ambientAirTemperature: 32, // AMBIENT_AIR_TEMP
        oilTemperature: 45, // ENGINE_OIL_TEMP
        coolantTemperature: 45, // ENGINE_COOLANT_TEMP
        intakeAirTemperature: 60, // INTAKE_AIR_TEMP

        // Progress bars
        engineLoad: 45, // ENGINE_LOAD
        relativeThrottlePosition: 45, // RELATIVE_THROTTLE_POSITION
        actualTorque: 60, // ACTUAL_ENGINE_TORQUE
      },
    };
  },
  mounted() {
    if (window.EventSource) {
      var source = new EventSource("/events");

      source.addEventListener(
        "open",
        function () {
          console.log("Events Connected");
        },
        false
      );

      source.addEventListener(
        "error",
        function (e) {
          if (e.target.readyState != EventSource.OPEN) {
            console.log("Events Disconnected");
            this.error = true;

            // Tenta di riconnetterti, altirmenti ricarica la pagina
          }
        },
        false
      );

      source.addEventListener(
        "dataupdate",
        (e) => this.updateEngine(e.data),
        false
      );
    }
  },
  methods: {
    updateEngine(data) {
      this.engine = JSON.parse(data);
    },
  },
};
</script>

<style>
/* Dark */

body {
  background-color: var(--bs-gray-dark);
  color: var(--bs-light);
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
  min-height: 100%; /* Fallback for browsers do NOT support vh unit */
  min-height: 100vh; /* These two lines are counted as one :-) */

  display: flex;
  align-items: center;
}
</style>
