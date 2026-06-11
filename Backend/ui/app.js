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

    // TODO : implement filters and filter by machine
    loadRuns();
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

function openRunDialog()
{
    document.getElementById("runDialog").style.display = "flex";
    loadRunDropdowns();
}

function closeRunDialog()
{
    document.getElementById("runDialog").style.display = "none";
}

async function submitRun()
{
    const payload = {
        machineId: +document.getElementById("rMachine").value,
        hardwareConfigurationId: +document.getElementById("rHw").value,
        softwareEnvironmentId: +document.getElementById("rEnv").value,
        softwareConfigurationId: +document.getElementById("rSoft").value,
        testId: +document.getElementById("rTest").value,
        testConfigurationId: +document.getElementById("rTestCfg").value,
        timestamp: new Date().toISOString(),
        result: {
            score: +document.getElementById("rScore").value,
            avgFps: +document.getElementById("rAvg").value,
            minFps: +document.getElementById("rMin").value,
            maxFps: +document.getElementById("rMax").value
        }
    };

    await fetch("/api/run/create", {
        method: "POST",
        headers: {"Content-Type": "application/json"},
        body: JSON.stringify(payload)
    });

    closeRunDialog();
    loadRuns();
}

async function loadRunDropdowns()
{
    const [machines, hw, env, soft, tests, testCfg] = await Promise.all([
        fetch("/api/list-machines").then(r => r.json()),
        fetch("/api/list-hardware-configs").then(r => r.json()),
        fetch("/api/list-software-environments").then(r => r.json()),
        fetch("/api/list-software-configs").then(r => r.json()),
        fetch("/api/list-tests").then(r => r.json()),
        fetch("/api/list-test-configs").then(r => r.json())
    ]);

    fillSelect("rMachine", machines.machines);
    fillSelect("rHw", hw.configs);
    fillSelect("rEnv", env.softwareEnvironments);
    fillSelect("rSoft", soft.softwareConfigurations);
    fillSelect("rTest", tests.tests);
    fillSelect("rTestCfg", testCfg.testConfigurations);
}

function fillSelect(id, items)
{
    const sel = document.getElementById(id);
    sel.innerHTML = "";

    for (const it of items)
    {
        const opt = document.createElement("option");
        opt.value = it.id;
        opt.textContent = it.name;
        sel.appendChild(opt);
    }
}

function openImportDialog()
{
    document.getElementById("importFile").click();
}

function handleImport(event)
{
    const file = event.target.files[0];
    if (!file) return;

    console.log("Selected file:", file.name);

    // later: parse + send to backend importer
}

async function loadRuns()
{
    const response = await fetch("/api/run/list");
    const data = await response.json();

    const tbody = document.getElementById("runsTableBody");
    tbody.innerHTML = "";

    for (const run of data.runs)
    {
        const r = run.result;

        const row = document.createElement("tr");

        row.innerHTML = `
            <td>${run.test}</td>
            <td>${run.testConfiguration}</td>
            <td>${r.score}</td>
            <td>${r.avgFps}</td>
            <td>${r.minFps}</td>
            <td>${r.maxFps}</td>
        `;

        tbody.appendChild(row);
    }
}

window.onload = () => {
    setView('home');
};
window.submitMachine = submitMachine;
window.closeMachineDialog = closeMachineDialog;
window.openMachineDialog = openMachineDialog;

window.openRunDialog = openRunDialog;
window.closeRunDialog = closeRunDialog;
window.submitRun = submitRun;
window.loadRunDropdowns = loadRunDropdowns;

window.openImportDialog = openImportDialog;
window.handleImport = handleImport;

