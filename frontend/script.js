// ------------------- Resources -------------------
function loadResources() {
    fetch('resources.txt')
        .then(res => res.text())
        .then(data => {
            const lines = data.split('\n');
            const table = document.getElementById('resourceTable');
            if (!table) return; // page may not have resources table
            table.innerHTML = '<tr><th>Resource</th><th>Status</th><th>Action</th></tr>';
            lines.forEach(line => {
                if (!line.trim()) return;
                const [name, avail] = line.split(',');
                const row = document.createElement('tr');
                row.innerHTML = `
                    <td>${name}</td>
                    <td>${parseInt(avail) > 0 ? 'Available' : 'Not Available'}</td>
                    <td>${parseInt(avail) > 0 ? '<button onclick="bookResource(\'' + name + '\')">Book</button>' : '<button disabled>Booked</button>'}</td>
                `;
                table.appendChild(row);
            });
        });
}

function bookResource(name) {
    alert(`Booking resource: ${name}\n(This requires backend to update resources.txt)`);
}

// ------------------- Navigation -------------------
const locations = ["Main Gate","Library","Admin Block","CSE Lab","Hostel","Canteen","Auditorium","Playground","Parking","Medical Center"];

function populateNavigationSelects() {
    const from = document.getElementById('fromLocation');
    const to = document.getElementById('toLocation');
    if (!from || !to) return;
    locations.forEach(loc => {
        const opt1 = document.createElement('option');
        opt1.value = loc; opt1.text = loc;
        from.appendChild(opt1);
        const opt2 = document.createElement('option');
        opt2.value = loc; opt2.text = loc;
        to.appendChild(opt2);
    });
}

function findPath(event) {
    event.preventDefault();
    const from = document.getElementById('fromLocation').value;
    const to = document.getElementById('toLocation').value;
    if (from === to) {
        alert("You are already at the destination!");
        return;
    }
    alert(`Shortest path from ${from} to ${to}\n(This requires backend logic to calculate actual path)`);
}

// ------------------- Login -------------------
function handleLogin(event) {
    event.preventDefault();
    const username = document.getElementById('username').value;
    const password = document.getElementById('password').value;
    if(username && password){
        alert(`Login as ${username}\n(This requires backend authentication)`);
        window.location.href = "dashboard.html";
    } else {
        alert("Please enter username and password.");
    }
}

// ------------------- Init -------------------
document.addEventListener('DOMContentLoaded', () => {
    loadResources();
    populateNavigationSelects();

    const loginForm = document.getElementById('loginForm');
    if(loginForm) loginForm.addEventListener('submit', handleLogin);

    const navForm = document.getElementById('navForm');
    if(navForm) navForm.addEventListener('submit', findPath);
});
