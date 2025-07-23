const dataLenght = 80*30; // 30s of data

const weightText = document.getElementById("weight-value");
const weightMaxText = document.getElementById("weight-max");
const batteryPercentageText = document.getElementById("battery-percentage");
const batteryFill = document.getElementById('battery-fill');
const confirmationModal = document.getElementById("confirmation-modal");
const confirmationMessage = document.getElementById("confirmation-message");
const graph = document.getElementById("Weight-Graph").getContext("2d");

const graph_labels = new Array(dataLenght);
const graph_data = {
    labels: graph_labels,
    datasets: [{
        label: 'Strenght (kg)',
        data: new Array(dataLenght).fill(0),
        borderColor: 'blue',
        backgroundColor: 'rgba(0, 0, 255, 0.1)',
        tension: 0.3,
    }]
};

const cgraph_onfig = {
    type: 'line',
    data: graph_data,
    options: {
        animation: false,
        responsive: true,
        maintainAspectRatio: false,
        plugins: {
            legend: {
            display: false
            }
        },
        elements: {
            point: {
            pointRadius: 0
            }
        },

        scales: {
            x: {
                display: false
            },
            y: {
                title: { display: false},
                beginAtZero: true,
                suggestedmin: 0,
                suggestedMax: 50
            }
        }
    }
};

const chart = new Chart(graph, cgraph_onfig);

// Getting data from web socket
const socket = new WebSocket(`ws://${location.host}/ws`);

socket.binaryType = 'arraybuffer';

socket.onmessage = function(event) {
    let len = (event.data.byteLength-1) / 12;

    let weights = new Float32Array(event.data.slice(0, 4*len));
    let timeStamp = new BigInt64Array(event.data.slice(4*len, 12*len));
    let batteryLevel = new Int8Array(event.data.slice(12*len, 12*len+1))[0];

    weightText.textContent = (weights[len-1]/1000).toFixed(1);
    batteryPercentageText.textContent = batteryLevel + "%";
    batteryFill.style.width = batteryLevel + "%";
    
    // Ajouter donnée au graphique
    for (let i = 0; i < len; i++) {
        graph_labels.push(timeStamp[i].toString());
        graph_data.datasets[0].data.push(weights[i]/1000);
    }

    while (graph_labels.length > dataLenght) {
        graph_labels.shift();
        graph_data.datasets[0].data.shift();
    }

    weightMaxText.textContent = Math.max(...graph_data.datasets[0].data).toFixed(1);


    chart.update();

};

socket.onerror = function(err) {
    console.log("Erreur WebSocket : ", err);
};

function openTab(evt, tabName) {
    var i, tabcontent, tablinks;
    tabcontent = document.getElementsByClassName("tab-content");
    for (i = 0; i < tabcontent.length; i++) {
        tabcontent[i].style.display = "none";
    }
    tablinks = document.getElementsByClassName("tab-button");
    for (i = 0; i < tablinks.length; i++) {
        tablinks[i].className = tablinks[i].className.replace(" active", "");
    }
    document.getElementById(tabName).style.display = "block";
    evt.currentTarget.className += " active";
}

document.addEventListener("DOMContentLoaded", function() {
    // Set the default active tab on page load
    document.querySelector(".tab-button").click();
});

document.getElementById("form-tarring").addEventListener("submit", function(event) {
    event.preventDefault();

    const formData = new FormData();
    formData.append('tare', '1');
    
    fetch("/config", { method: 'POST', body: new URLSearchParams(formData)})
        .then(response => response.text())
        .then(text => {            
            showConfirmation(text);
        })
        .catch(error => {
            console.error('Error:', error);
        });
});

document.getElementById("form-configuration").addEventListener("submit", function(event) {
    event.preventDefault();

    const formData = new FormData();
    formData.append('device_name', document.getElementById("device_name").value);
    formData.append('scale_factor', document.getElementById("scale_factor").value);
    formData.append('startup_mode', document.getElementById("startup_mode").value);

    fetch("/config", { method: 'POST', body: new URLSearchParams(formData)})
    .then(response => response.text())
    .then(text => {        
        showConfirmation(text);
    })
    .catch(error => {
        console.error('Error:', error);
    });
});


document.getElementById("form-password").addEventListener("submit", function(event) {
    event.preventDefault();

    const formData = new FormData();
    formData.append('old_password', document.getElementById("old_password").value);
    formData.append('new_password', document.getElementById("new_password").value);
    formData.append('new_password_bis', document.getElementById("new_password_bis").value);

    fetch("/config", { method: 'POST', body: new URLSearchParams(formData)})
    .then(response => response.text())
    .then(text => {        
        showConfirmation(text);
    })
    .catch(error => {
        console.error('Error:', error);
    });
});


document.getElementById("form-calibration").addEventListener("submit", function(event) {
    event.preventDefault();

    const formData = new FormData();
    formData.append('Calibration', document.getElementById("Calibration").value);

    fetch("/config", { method: 'POST', body: new URLSearchParams(formData)})
    .then(response => response.text())
    .then(text => {        
        showConfirmation(text);
    })
    .catch(error => {
        console.error('Error:', error);
    });
});

function showConfirmation(message) {
    confirmationMessage.textContent = message;
    confirmationModal.style.display = "block";

    // Close the modal after a few seconds (e.g., 3 seconds)
    setTimeout(function() {
        confirmationModal.style.display = "none";
    }, 5000);
}
