const SERVO_COUNT = 6;
let renderedSequenceCount = 0;

function startMove(id, dir) {
    fetch(`/startMove?servo=${id}&dir=${dir}`);
}

function stopMove(id) {
    fetch(`/stopMove?servo=${id}`);
}

function stopAll() {
    for (let i = 0; i < SERVO_COUNT; i++) {
        fetch(`/stopMove?servo=${i}`);
    }
}

function recordPosition() {
    fetch("/record")
        .then((res) => res.text())
        .then(() => refreshSequenceData())
        .catch((err) => console.error("Fehler bei /record:", err));
}

function overwriteSequence() {
    fetch("/overwriteSequence")
        .then((res) => {
            if (!res.ok) {
                return res.text().then((text) => Promise.reject(new Error(text)));
            }
            return res.text();
        })
        .then(() => refreshSequenceData())
        .catch((err) => console.error("Fehler bei /overwriteSequence:", err));
}

function playSequence() {
    fetch("/playSequence")
        .then((res) => {
            if (!res.ok) {
                return res.text().then((text) => Promise.reject(new Error(text)));
            }
            return res.text();
        })
        .then(() => refreshSequenceData())
        .catch((err) => console.error("Fehler bei /playSequence:", err));
}

function clearSequence() {
    fetch("/clearSequence")
        .then((res) => res.text())
        .then(() => refreshSequenceData())
        .catch((err) => console.error("Fehler bei /clearSequence:", err));
}

function moveHome() {
    fetch("/home")
        .then(() => refreshSequenceData())
        .catch((err) => console.error("Fehler bei /home:", err));
}

function stopSequence() {
    fetch("/stop")
        .then((res) => res.text())
        .then(() => refreshSequenceData())
        .catch((err) => console.error("Fehler bei /stop:", err));
}

function updateSpeedLabel(id, value) {
    document.getElementById(`speedVal${id}`).innerText = value;
}

function syncSpeedControls(data) {
    for (let i = 0; i < data.length; i++) {
        const slider = document.getElementById(`speed${i}`);
        if (!slider) {
            continue;
        }
        slider.value = data[i];
        updateSpeedLabel(i, data[i]);
    }
}

function setServoSpeed(id, value) {
    fetch(`/setSpeed?servo=${id}&value=${value}`)
        .then((res) => {
            if (!res.ok) {
                return res.text().then((text) => Promise.reject(new Error(text)));
            }
            return refreshSpeeds();
        })
        .catch((err) => console.error("Fehler bei /setSpeed:", err));
}

function initControlBindings() {
    document.querySelectorAll(".speed-slider").forEach((slider) => {
        slider.addEventListener("input", (event) => {
            const id = Number(event.target.dataset.servo);
            updateSpeedLabel(id, event.target.value);
        });
        slider.addEventListener("change", (event) => {
            const id = Number(event.target.dataset.servo);
            setServoSpeed(id, event.target.value);
        });
    });

    document.querySelectorAll(".minus, .plus").forEach((button) => {
        const startHandler = (event) => {
            event.preventDefault();
            const id = Number(button.dataset.servo);
            const dir = Number(button.dataset.dir);
            startMove(id, dir);
        };
        const stopHandler = (event) => {
            event.preventDefault();
            const id = Number(button.dataset.servo);
            stopMove(id);
        };

        button.addEventListener("mousedown", startHandler);
        button.addEventListener("mouseup", stopHandler);
        button.addEventListener("mouseleave", stopHandler);
        button.addEventListener("touchstart", startHandler, { passive: false });
        button.addEventListener("touchend", stopHandler);
        button.addEventListener("touchcancel", stopHandler);
    });
}

function renderSequenceList(data) {
    const list = document.getElementById("sequenceList");
    renderedSequenceCount = data.count;
    if (!data.steps.length) {
        list.innerHTML = '<p class="sequence-empty">Noch keine Positionen gespeichert.</p>';
        return;
    }

    list.innerHTML = data.steps.map((step, index) => {
        const positions = step.map((value, servoIndex) =>
            `<div>Servo ${servoIndex}: ${value}${String.fromCharCode(176)}</div>`
        ).join("");
        const activeClass = data.currentStep === index ? " active" : "";
        const stateLabel = data.currentStep === index ? " (aktiv)" : "";
        return `
            <div class="sequence-step${activeClass}" data-step-index="${index}">
                <div class="sequence-step-title">Schritt ${index + 1}${stateLabel}</div>
                <div class="sequence-step-grid">${positions}</div>
            </div>
        `;
    }).join("");
}

function updateSequenceStatus(data) {
    document.getElementById("sequenceStatus").innerText =
        `Gespeicherte Positionen: ${data.count} | Wiedergabe: ${data.playing ? "aktiv" : "bereit"}`;
}

function setActiveSequenceStep(currentStep) {
    const steps = document.querySelectorAll(".sequence-step");
    steps.forEach((step) => {
        const index = Number(step.dataset.stepIndex);
        const title = step.querySelector(".sequence-step-title");
        const isActive = index === currentStep;
        step.classList.toggle("active", isActive);
        title.innerText = `Schritt ${index + 1}${isActive ? " (aktiv)" : ""}`;
    });
}

function refreshSequenceData() {
    return fetch("/sequenceData")
        .then((res) => res.json())
        .then((data) => {
            updateSequenceStatus(data);
            renderSequenceList(data);
        })
        .catch((err) => console.error("Fehler bei /sequenceData:", err));
}

function refreshSequenceStatus() {
    fetch("/sequenceStatus")
        .then((res) => res.json())
        .then((data) => {
            updateSequenceStatus(data);
            setActiveSequenceStep(data.currentStep);
            if (data.count !== renderedSequenceCount) {
                return refreshSequenceData();
            }
        })
        .catch((err) => console.error("Fehler bei /sequenceStatus:", err));
}

function refreshPositions() {
    fetch("/positions")
        .then((res) => res.json())
        .then((data) => {
            for (let i = 0; i < data.length; i++) {
                document.getElementById(`val${i}`).innerText =
                    `${data[i]} ${String.fromCharCode(176)}`;
            }
        })
        .catch((err) => console.error("Fehler bei /positions:", err));
}

function refreshSpeeds() {
    return fetch("/speeds")
        .then((res) => res.json())
        .then((data) => syncSpeedControls(data))
        .catch((err) => console.error("Fehler bei /speeds:", err));
}

setInterval(refreshPositions, 300);
refreshPositions();
setInterval(refreshSequenceStatus, 500);
refreshSequenceData();
refreshSpeeds();
initControlBindings();