<template>
  <div :class="[ smallWidth, normalWidth, 'gy-3']">
    <slot></slot>
  </div>
</template>

<script>
export default {
  name: "ContainerSlot",
  props: ["width", "is_small"],
  computed: {
    normalWidth() {
      return "col-sm-" + (this.width ? this.width : "4");
    },
    smallWidth() {
      return this.is_small === "" ? "col-6" : "col-12";
    },
  },
};
</script>