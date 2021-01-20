<template>
  <div class="col text-center" v-if="typeof this.level != 'undefined'">
    <div class="progress">
      <div
        :class="[state, 'progress-bar']"
        role="progressbar"
        :style="{ width: normalized + '%' }"
        :aria-valuenow="normalized"
        :aria-valuemin="min"
        :aria-valuemax="max"
      ></div>
    </div>
    <span v-if="unit === ''" class="mt-1 d-block">{{ name }}: {{ level }}</span>
    <span v-else class="mt-1 d-block"
      >{{ name }} ({{ unit }}): {{ level }}</span
    >
  </div>
  <div class="col-4 text-center" v-else>{{ name }} not available</div>
</template>

<script>
export default {
  name: "ProgressBar",
  props: ["name", "unit", "is_graded", "level", "min", "max"],
  computed: {
    state() {
      if (!this.is_graded) return "";
      if (this.normalized >= 90) return "bg-danger";
      if (this.normalized >= 70) return "bg-warning";
      return "bg-success";
    },
    normalized() {
      // Level "normalisation": [min, max] -> [0, 100]
      return Math.round(
        ((this.level - this.min) * 100) / (this.max - this.min)
      );
    },
  },
};
</script>