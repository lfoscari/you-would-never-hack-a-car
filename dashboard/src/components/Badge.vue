<template>
  <div class="text-center">
    <span class="display-1 font-weight-bold">{{ value }}</span>
    <br />
    <p class="mb-0">{{ unit }}</p>
  </div>
</template>

<script>
export default {
  name: "Badge",
  props: ["name", "unit", "value"],
};
</script>