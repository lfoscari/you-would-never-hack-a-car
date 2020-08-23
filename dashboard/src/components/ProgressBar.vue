<template>
  <div>
    <span v-if="unit === ''" class="mb-1 d-block">{{ name }}: {{ level }}</span>
    <span v-else class="mb-1 d-block">{{ name }} ({{ unit }}): {{ level }}</span>
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
  </div>
</template>

<script>
export default {
  name: "ProgressBar",
  props: ["name", "unit", "is_graded", "level", "min", "max"],
  computed: {
    state() {
      if (!this.is_graded) return "";
      if (this.normalized > 90) return "bg-warning";
      if (this.normalized > 70) return "bg-danger";
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