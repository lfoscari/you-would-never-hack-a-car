<template>
  <div class="text-center">
    <span class="display-1 font-weight-bold">{{ value }}</span>
    <br />
    <p class="mb-0">{{ label }}</p>
  </div>
</template>

<script>
export default {
  name: "Badge",
  props: ["label", "value"],
};
</script>

  <!-- <span class="nes-badge mr-3 is-splited">
    <span class="is-dark">{{ label }}</span>
    <span v-if="!is_binary || value == 'on'" class="is-primary">{{ value }}</span>
    <span v-else class="is-warning">{{ value }}</span>
  </span> -->