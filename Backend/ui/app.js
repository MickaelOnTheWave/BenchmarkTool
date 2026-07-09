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

     if (view === "details")
     {
         await loadDetailsTables();
     }
}

const detailsTableDefinitions = [
    {
        id: "machines",
        title: "Machines",
        endpoint: "/api/list-machines",
        rootField: "machines",
        columns: [
            { key: "id", label: "Id" },
            { key: "name", label: "Name" },
            { key: "cpu", label: "Cpu" },
            { key: "gpu", label: "Gpu" },
            { key: "ramGb", label: "RamGb" },
            { key: "motherboard", label: "Motherboard" }
        ]
    },
    {
        id: "hardwareConfigs",
        title: "Hardware Configs",
        endpoint: "/api/list-hardware-configs",
        rootField: "configs",
        columns: [
            { key: "id", label: "Id" },
            { key: "name", label: "Name" },
            { key: "machineId", label: "MachineId" },
            { key: "cpuFreqGhz", label: "CpuFreqGhz" },
            { key: "gpuFreqMhz", label: "GpuFreqMhz" },
            { key: "ramFreqMhz", label: "RamFreqMhz" },
            { key: "settings", label: "Settings" }
        ]
    },
    {
        id: "softwareEnvironments",
        title: "Software Environments",
        endpoint: "/api/list-software-environments",
        rootField: "softwareEnvironments",
        columns: [
            { key: "id", label: "Id" },
            { key: "name", label: "Name" },
            { key: "os", label: "Os" },
            { key: "osVersion", label: "OsVersion" },
            { key: "driverFamily", label: "DriverFamily" }
        ]
    },
    {
        id: "softwareConfigs",
        title: "Software Configs",
        endpoint: "/api/list-software-configs",
        rootField: "softwareConfigurations",
        columns: [
            { key: "id", label: "Id" },
            { key: "name", label: "Name" },
            { key: "softwareEnvironmentId", label: "SoftwareEnvironmentId" },
            { key: "driverVersion", label: "DriverVersion" },
            { key: "mode", label: "Mode" },
            { key: "settings", label: "Settings" }
        ]
    },
    {
        id: "tests",
        title: "Tests",
        endpoint: "/api/list-tests",
        rootField: "tests",
        columns: [
            { key: "id", label: "Id" },
            { key: "name", label: "Name" },
            { key: "description", label: "Description" },
            { key: "iconPath", label: "IconPath" }
        ]
    },
    {
        id: "testConfigs",
        title: "Test Configs",
        endpoint: "/api/list-test-configs",
        rootField: "testConfigurations",
        columns: [
            { key: "id", label: "Id" },
            { key: "name", label: "Name" },
            { key: "testId", label: "TestId" },
            { key: "settings", label: "Settings" }
        ]
    }
];

async function loadDetailsTables()
{
    const container = document.getElementById("detailsTables");
    container.innerHTML = "";

    for (const definition of detailsTableDefinitions)
    {
        container.appendChild(createDetailsTable(definition));
        await refreshDetailsTable(definition);
    }
}

function createDetailsTable(definition)
{
    const panel = document.createElement("section");
    panel.className = "details-table-panel";

    const header = document.createElement("div");
    header.className = "details-table-header";

    const title = document.createElement("h3");
    title.textContent = definition.title;

    const actions = document.createElement("div");
    actions.className = "details-table-actions";

    const addButton = document.createElement("button");
    addButton.type = "button";
    addButton.textContent = "Add";
    addButton.onclick = () => showDetailsPlaceholder(definition, "Add");

    const removeButton = document.createElement("button");
    removeButton.type = "button";
    removeButton.textContent = "Remove";
    removeButton.className = "danger-button";
    removeButton.disabled = true;
    removeButton.onclick = () => showDetailsPlaceholder(definition, "Remove");

    actions.appendChild(addButton);
    actions.appendChild(removeButton);
    header.appendChild(title);
    header.appendChild(actions);

    const tableWrap = document.createElement("div");
    tableWrap.className = "details-table-wrap";

    const table = document.createElement("table");
    table.id = `${definition.id}Table`;

    const thead = document.createElement("thead");
    const headerRow = document.createElement("tr");
    for (const column of definition.columns)
    {
        const th = document.createElement("th");
        th.textContent = column.label;
        headerRow.appendChild(th);
    }
    thead.appendChild(headerRow);

    const tbody = document.createElement("tbody");
    table.appendChild(thead);
    table.appendChild(tbody);
    tableWrap.appendChild(table);

    const status = document.createElement("div");
    status.id = `${definition.id}Status`;
    status.className = "details-table-status";

    panel.appendChild(header);
    panel.appendChild(tableWrap);
    panel.appendChild(status);

    return panel;
}

async function refreshDetailsTable(definition)
{
    const table = document.getElementById(`${definition.id}Table`);
    const tbody = table.querySelector("tbody");
    const panel = table.closest(".details-table-panel");
    const removeButton = panel.querySelector(".danger-button");
    const status = document.getElementById(`${definition.id}Status`);

    tbody.innerHTML = "";
    removeButton.disabled = true;
    panel.dataset.selectedId = "";
    status.textContent = "Loading...";

    try
    {
        const response = await fetch(definition.endpoint);
        const data = await response.json();
        const rows = data[definition.rootField] || [];

        for (const row of rows)
        {
            tbody.appendChild(createDetailsTableRow(definition, row, panel, removeButton));
        }

        status.textContent = rows.length === 0 ? "No entries." : `${rows.length} entries.`;
    }
    catch (error)
    {
        status.textContent = `Could not load ${definition.title}.`;
        console.error(error);
    }
}

function createDetailsTableRow(definition, row, panel, removeButton)
{
    const tr = document.createElement("tr");
    tr.tabIndex = 0;
    tr.onclick = () => selectDetailsRow(panel, tr, row.id, removeButton);

    for (const column of definition.columns)
    {
        const td = document.createElement("td");
        td.textContent = formatDetailsValue(row[column.key]);
        tr.appendChild(td);
    }

    return tr;
}

function selectDetailsRow(panel, rowElement, selectedId, removeButton)
{
    for (const row of panel.querySelectorAll("tbody tr"))
        row.classList.remove("selected-row");

    rowElement.classList.add("selected-row");
    panel.dataset.selectedId = selectedId;
    removeButton.disabled = false;
}

function showDetailsPlaceholder(definition, action)
{
    const panel = document.getElementById(`${definition.id}Table`).closest(".details-table-panel");
    const selectedId = panel.dataset.selectedId;

    if (action === "Remove" && !selectedId)
    {
        alert("Select a row to remove first.");
        return;
    }

    const suffix = action === "Remove" ? ` for id ${selectedId}` : "";
    alert(`${action} ${definition.title}${suffix}: backend wiring will be added later.`);
}

function formatDetailsValue(value)
{
    if (value === null || value === undefined || value === "")
        return "-";

    if (typeof value === "object")
        return JSON.stringify(value);

    return String(value);
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

async function handleImport(event)
{
    const file = event.target.files[0];
    if (!file) return;

    console.log("Selected file:", file.name);

    const result = await uploadFiles([file]);

    console.log("Import result:", result);

    // optional: reset input so same file can be selected again
    event.target.value = "";
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

async function uploadFiles(files)
{
    const formData = new FormData();

    for (const f of files)
        formData.append("file", f);

    const res = await fetch("/api/import/files", {
        method: "POST",
        body: formData
    });

    return await res.json();
}

window.onload = () => {
    setView('home');
};
window.setView = setView;
window.submitMachine = submitMachine;
window.closeMachineDialog = closeMachineDialog;
window.openMachineDialog = openMachineDialog;

window.openRunDialog = openRunDialog;
window.closeRunDialog = closeRunDialog;
window.submitRun = submitRun;
window.loadRunDropdowns = loadRunDropdowns;

window.openImportDialog = openImportDialog;
window.handleImport = handleImport;
