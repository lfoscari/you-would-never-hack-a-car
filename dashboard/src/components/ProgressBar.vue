<template>
  <div>
    <div v-if="level === null || max === null">
      <p>Your OBD II doesn't provide this informations</p>
    </div>
    <div v-else>
      <span class="mb-1 d-block">{{ label }}: {{ level }}</span>
      <div class="progress">
        <div
          :class="[state, 'progress-bar']"
          role="progressbar"
          :style="{ width: norm + '%' }"
          :aria-valuenow="level"
          :aria-valuemin="min"
          :aria-valuemax="max"
        ></div>
      </div>
    </div>
  </div>
</template>

<script>
export default {
  name: "ProgressBar",
  props: ["level", "min", "max", "label", "is_graded"],
  computed: {
    state() {
      if (!this.is_graded) return "";
      if (this.norm > 90) return "bg-warning";
      if (this.norm > 70) return "bg-danger";
      return "bg-success";
    },
    norm() {
      // Level "normalisation": [min, max] -> [0, 100]
      return Math.round(
        ((this.level - this.min) * 100) / (this.max - this.min)
      );
    },
  },
};
</script>