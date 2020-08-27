<template>
  <div id="app" class="container-md vertical-center overflow-hidden">
    <div v-if="error">
      <h1>Error reading the data file</h1>
    </div>
    <div v-else>
      <div class="row justify-content-center">
        <div v-if="typeof engine.gear != 'undefined'" class="col-sm-3 col-6 gy-3">
          <badge name="current gear" unit="Gear" :value="engine.gear"></badge>
        </div>

        <div v-if="typeof engine.lights != 'undefined'" class="col-sm-3 col-6 gy-3">
          <badge name="lights status" unit="Lights" :value="engine.lights ? 'on' : 'off'"></badge>
        </div>

        <div v-if="typeof engine.air_temperature != 'undefined'" class="col-sm-3 col-6 gy-3">
          <badge name="current temperature" unit="C°" :value="engine.air_temperature"></badge>
        </div>

        <div v-if="typeof engine.fuel != 'undefined'" class="col-sm-3 col-6 gy-3">
          <badge name="current fuel level" unit="Fuel" :value="engine.fuel"></badge>
        </div>
      </div>

      <div class="row justify-content-center">
        <div v-if="typeof engine.throttle != 'undefined'" class="col-sm-4 col-12 gy-3">
          <progress-bar
            name="Throttle position"
            unit="%"
            :level="engine.throttle"
            :min="0"
            :max="100"
          ></progress-bar>
        </div>

        <div
          v-if="typeof engine.oil_temp != 'undefined' && typeof engine.critical_oil_temp != 'undefined'"
          class="col-sm-4 col-12 gy-3"
        >
          <progress-bar
            name="Oil temperature"
            unit="C°"
            :level="engine.oil_temp"
            :min="0"
            :max="engine.critical_oil_temp"
            is_graded="true"
          ></progress-bar>
        </div>

        <div
          v-if="typeof engine.rpm != 'undefined' && typeof engine.max_rpm != 'undefined'"
          class="col-sm-4 col-12 gy-3"
        >
          <progress-bar
            name="RPM"
            unit
            :level="engine.rpm"
            :min="0"
            :max="engine.max_rpm"
            is_graded="true"
          ></progress-bar>
        </div>

        <div v-if="typeof engine.throttle != 'undefined'" class="col-sm-4 col-12 gy-3">
          <progress-bar
            name="Throttle position"
            unit="%"
            :level="engine.throttle"
            :min="0"
            :max="100"
          ></progress-bar>
        </div>

        <div v-if="typeof engine.throttle != 'undefined'" class="col-sm-4 col-12 gy-3">
          <progress-bar
            name="Throttle position"
            unit="%"
            :level="engine.throttle"
            :min="0"
            :max="100"
          ></progress-bar>
        </div>

        <div v-if="typeof engine.throttle != 'undefined'" class="col-sm-4 col-12 gy-3">
          <progress-bar
            name="Throttle position"
            unit="%"
            :level="engine.throttle"
            :min="0"
            :max="100"
          ></progress-bar>
        </div>

        <div class="col-12 gy-3">
          <p>Lorem ipsum dolor sit amet consectetur adipisicing elit. Fugit atque fugiat mollitia iste perferendis doloremque quos nihil ratione quasi aperiam provident a, labore inventore vel amet minus ea sapiente ab.</p>
        </div>
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
        gear: 0,
        lights: false,
        throttle: 0,
        oil_temp: 0,
        critical_oil_temp: 100,
        rpm: 0,
        max_rpm: 100,
        air_temperature: 0,
        fuel: 0,
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
