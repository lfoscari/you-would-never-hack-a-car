<template>
  <div id="app" class="container-md vertical-center overflow-hidden">
    <div v-if="error">
      <h1>Error reading the data file</h1>
    </div>
    <div v-else>
      <div v-if="loading">
        <h1>Loading...</h1>
      </div>
      <div v-else class="row">
        <ContainerSlot width="3" is_small>
          <Badge label="Gear" :value="engine.gear"></Badge>
        </ContainerSlot>

        <ContainerSlot width="3" is_small>
          <Badge label="Lights" :value="engine.lights ? 'on' : 'off'"></Badge>
        </ContainerSlot>

        <ContainerSlot width="3" is_small>
          <Badge label="C°" :value="engine.air_temperature"></Badge>
        </ContainerSlot>

        <ContainerSlot width="3" is_small>
          <Badge label="Fuel" :value="engine.fuel"></Badge>
        </ContainerSlot>

        <ContainerSlot>
          <ProgressBar label="Throttle (%)" :level="engine.throttle" :min="0" :max="100"></ProgressBar>
        </ContainerSlot>

        <ContainerSlot>
          <ProgressBar
            label="Oil temperature (C°)"
            :level="engine.oil_temp"
            :min="0"
            :max="engine.critical_oil_temp"
            is_graded="true"
          ></ProgressBar>
        </ContainerSlot>

        <ContainerSlot>
          <ProgressBar
            label="RPM"
            :level="engine.rpm"
            :min="0"
            :max="engine.max_rpm"
            is_graded="true"
          ></ProgressBar>
        </ContainerSlot>

        <ContainerSlot>
          <ProgressBar label="Throttle (%)" :level="engine.throttle" :min="0" :max="100"></ProgressBar>
        </ContainerSlot>

        <ContainerSlot>
          <ProgressBar label="Throttle (%)" :level="engine.throttle" :min="0" :max="100"></ProgressBar>
        </ContainerSlot>

        <ContainerSlot>
          <ProgressBar label="Throttle (%)" :level="engine.throttle" :min="0" :max="100"></ProgressBar>
        </ContainerSlot>

        <ContainerSlot width="12">
          <p>Lorem ipsum dolor sit amet consectetur adipisicing elit. Fugit atque fugiat mollitia iste perferendis doloremque quos nihil ratione quasi aperiam provident a, labore inventore vel amet minus ea sapiente ab.</p>
        </ContainerSlot>
      </div>
    </div>
  </div>
</template>

<script>
import axios from "axios";

import ContainerSlot from "./components/ContainerSlot.vue";
import ProgressBar from "./components/ProgressBar.vue";
import Badge from "./components/Badge.vue";

let ENGINE_DATA = "./data.json";
let REFRESH_TIMER = 1000;

export default {
  name: "App",
  components: {
    ContainerSlot,
    ProgressBar,
    Badge,
  },
  data() {
    return {
      loading: true,
      error: false,
      engine: null,
    };
  },
  mounted() {
    setInterval(() => {
      this.update();
    }, REFRESH_TIMER);
  },
  methods: {
    update() {
      axios
        .get(ENGINE_DATA)
        .then((response) => (this.engine = response.data))
        .catch((error) => {
          console.log(error);
          this.error = true;
        })
        .finally(() => (this.loading = false));
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
}

.vertical-center {
  min-height: 100%; /* Fallback for browsers do NOT support vh unit */
  min-height: 100vh; /* These two lines are counted as one :-) */

  display: flex;
  align-items: center;
}
</style>
