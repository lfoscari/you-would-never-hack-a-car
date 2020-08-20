<template>
  <div id="app" class="container my-5">
    <div class="row">
      <ContainerSlot>
        <a class="nes-badge mr-3 is-splited">
          <span class="is-dark">gear</span>
          <span class="is-primary">
            5
            <!-- <sup>th</sup> -->
          </span>
        </a>

        <a class="nes-badge mr-3">
          <span class="is-success">Lights: on</span>
        </a>
      </ContainerSlot>

      <ContainerSlot>
        <p>Throttle position: 25%</p>
        <progress class="nes-progress" value="25" max="100"></progress>
      </ContainerSlot>

      <ContainerSlot>
        <p>Oil temperature: 85°</p>
        <progress class="nes-progress is-error" value="85" max="100"></progress>
      </ContainerSlot>

      <ContainerSlot>
        <p>RPM: 2000</p>
        <progress class="nes-progress is-success" value="2000" max="5000"></progress>
      </ContainerSlot>

      <ContainerSlot>
        <p>RPM: 3000</p>
        <progress class="nes-progress is-warning" value="3000" max="5000"></progress>
      </ContainerSlot>

      <ContainerSlot>
        <p>RPM: 4500</p>
        <progress class="nes-progress is-error" value="4500" max="5000"></progress>
      </ContainerSlot>
    </div>
  </div>
</template>

<script>
import ContainerSlot from "./components/ContainerSlot.vue";

export default {
  name: "App",
  components: {
    ContainerSlot,
  },
};
</script>

<style lang="scss">
@import url("assets/bootstrap-grid.min.css");
</style>
