<template>
  <div
    class="col-6 text-center position-relative"
    v-if="typeof value != 'undefined'"
  >
    <div class="angle-container">
      <div class="angle" v-bind:style="{ transform: rotation }"></div>
    </div>
    <span class="display-1 font-weight-bold">{{ value }}°</span>
    <p class="mb-0">{{ name }}</p>
  </div>
  <div class="col-6 text-center position-relative" v-else>
    {{ name }} not available
  </div>
</template>

<script>
export default {
  name: "Tilt",
  props: ["name", "value"],
  computed: {
    rotation() {
      return `rotate(${this.value}deg)`;
    },
  },
};
</script>

<style>
:root {
  --size: 40vw;
}

.angle-container {
  position: absolute;
  z-index: -10;

  top: 50%;
  margin-top: calc(-1 * var(--size) / 2);

  left: 50%;
  margin-left: calc(-1 * var(--size) / 2);

  height: var(--size);
  width: var(--size);

  opacity: 0.2;
  background-color: var(--bs-gray);
  border-radius: 100%;
}

.angle {
  position: relative;
  top: 50%;

  transform-origin: top center;
  transition: transform 0.5s linear;

  height: 50%;
  width: 100%;

  background-color: var(--bs-white);
  border-radius: 0 0 calc(var(--size) / 2) calc(var(--size) / 2);
}
</style>