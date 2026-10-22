const brightness = document.getElementById("brightness");
const brightnessValue = document.getElementById("brightnessValue");

brightness.addEventListener("input", () => {
    brightnessValue.textContent = brightness.value;
});