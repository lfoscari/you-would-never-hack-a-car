<template>
  <div class="col text-center" v-if="typeof value != 'undefined'">
    <span class="display-1 font-weight-bold">{{ value }}</span>
    <small v-if="unit">{{ unit }}</small>
    <p class="mb-0">{{ name }}</p>
  </div>
  <div class="col text-center" v-else>{{ name }} not available</div>
</template>

<script>
export default {
  name: "Badge",
  props: ["name", "unit", "value"],
};
</script>