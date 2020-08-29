<template>
  <div class="text-center">
    <span class="display-1 font-weight-bold">{{ value }}</span>
    <small v-if="unit">{{ unit }}</small>
    <p class="mb-0">{{ name }}</p>
  </div>
</template>

<script>
export default {
  name: "Badge",
  props: ["name", "unit", "value"],
};
</script>