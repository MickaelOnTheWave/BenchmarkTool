async function setView(view)
{
   const sections =
   [
       "homeSection",
       "machineStatsSection",
       "configStatsSection",
       "detailsSection",
       "settingsSection",
       "aboutSection"
   ];

   for (const id of sections)
       document.getElementById(id).style.display = "none";

   document.getElementById(view + "Section").style.display = "block";

    if (view === "home")
    {
        await loadMachines();
        document.getElementById("addMachineBtn").onclick = openMachineDialog;
    }
}

async function loadMachines()
{
  const response =
      await fetch("http://localhost:8080/api/list-machines");

  const data = await response.json();

  const list = document.getElementById("machineList");
  list.innerHTML = "";

  for (const machine of data.machines)
  {
      const item = document.createElement("li");

      item.textContent = machine.name;
      item.style.cursor = "pointer";
      item.style.padding = "4px";

      item.onclick = () =>
      {
          selectMachine(machine);
      };

      list.appendChild(item);
  }

  if (data.machines.length === 1)
  {
    selectMachine(data.machines[0]);
  }
}

function selectMachine(machine)
{
  const details = document.getElementById("machineDetails");

  details.innerHTML = `
      <table>
          <tr><td><b>CPU</b></td><td>${machine.cpu}</td></tr>
          <tr><td><b>GPU</b></td><td>${machine.gpu}</td></tr>
          <tr><td><b>RAM</b></td><td>${machine.ramGb} GB</td></tr>
          <tr><td><b>Motherboard</b></td><td>${machine.motherboard}</td></tr>
      </table>
  `;

  // later:
  // loadBenchmarkRuns(machine.id);
}

function openMachineDialog()
{
    document.getElementById("addMachineDialog").style.display = "flex";
}

function closeMachineDialog()
{
    document.getElementById("addMachineDialog").style.display = "none";
}

async function submitMachine()
{
    const payload = {
        name: document.getElementById("mName").value,
        cpu: document.getElementById("mCpu").value,
        gpu: document.getElementById("mGpu").value,
        ramGb: parseInt(document.getElementById("mRam").value || "0"),
        motherboard: document.getElementById("mMotherboard").value
    };

    await fetch("/api/create-machine", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(payload)
    });

    closeMachineDialog();
    loadMachines();
}

window.onload = () => {
    setView('home');
};
window.submitMachine = submitMachine;
window.closeMachineDialog = closeMachineDialog;
window.openMachineDialog = openMachineDialog;
