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
        createEndpoint: "/api/create-machine",
        deleteEndpoint: "/api/delete-machine",
        rootField: "machines",
        columns: [
            { key: "id", label: "Id" },
            { key: "name", label: "Name" },
            { key: "cpu", label: "Cpu" },
            { key: "gpu", label: "Gpu" },
            { key: "ramGb", label: "RamGb" },
            { key: "motherboard", label: "Motherboard" }
        ],
        fields: [
            { key: "name", label: "Name", type: "text" },
            { key: "cpu", label: "Cpu", type: "text" },
            { key: "gpu", label: "Gpu", type: "text" },
            { key: "ramGb", label: "RamGb", type: "number" },
            { key: "motherboard", label: "Motherboard", type: "text" }
        ]
    },
    {
        id: "hardwareConfigs",
        title: "Hardware Configs",
        endpoint: "/api/list-hardware-configs",
        createEndpoint: "/api/create-hardware-config",
        deleteEndpoint: "/api/delete-hardware-config",
        rootField: "configs",
        columns: [
            { key: "id", label: "Id" },
            { key: "name", label: "Name" },
            { key: "machineId", label: "MachineId" },
            { key: "cpuFreqGhz", label: "CpuFreqGhz" },
            { key: "gpuFreqMhz", label: "GpuFreqMhz" },
            { key: "ramFreqMhz", label: "RamFreqMhz" },
            { key: "settings", label: "Settings" }
        ],
        fields: [
            { key: "name", label: "Name", type: "text" },
            {
                key: "machineId",
                label: "Machine",
                type: "select",
                options: { endpoint: "/api/list-machines", rootField: "machines", valueField: "id", displayField: "name" }
            },
            { key: "cpuFreqGhz", label: "CpuFreqGhz", type: "number" },
            { key: "gpuFreqMhz", label: "GpuFreqMhz", type: "number" },
            { key: "ramFreqMhz", label: "RamFreqMhz", type: "number" },
            { key: "settings", label: "Settings", type: "textarea", defaultValue: "{}" }
        ]
    },
    {
        id: "softwareEnvironments",
        title: "Software Environments",
        endpoint: "/api/list-software-environments",
        createEndpoint: "/api/create-software-environment",
        deleteEndpoint: "/api/delete-software-environment",
        rootField: "softwareEnvironments",
        columns: [
            { key: "id", label: "Id" },
            { key: "name", label: "Name" },
            { key: "os", label: "Os" },
            { key: "osVersion", label: "OsVersion" },
            { key: "driverFamily", label: "DriverFamily" }
        ],
        fields: [
            { key: "name", label: "Name", type: "text" },
            { key: "os", label: "Os", type: "text" },
            { key: "osVersion", label: "OsVersion", type: "text" },
            { key: "driverFamily", label: "DriverFamily", type: "text" }
        ]
    },
    {
        id: "softwareConfigs",
        title: "Software Configs",
        endpoint: "/api/list-software-configs",
        createEndpoint: "/api/create-software-config",
        deleteEndpoint: "/api/delete-software-config",
        rootField: "softwareConfigurations",
        columns: [
            { key: "id", label: "Id" },
            { key: "name", label: "Name" },
            { key: "softwareEnvironmentId", label: "SoftwareEnvironmentId" },
            { key: "driverVersion", label: "DriverVersion" },
            { key: "mode", label: "Mode" },
            { key: "settings", label: "Settings" }
        ],
        fields: [
            { key: "name", label: "Name", type: "text" },
            {
                key: "softwareEnvironmentId",
                label: "Software Environment",
                type: "select",
                options: { endpoint: "/api/list-software-environments", rootField: "softwareEnvironments", valueField: "id", displayField: "name" }
            },
            { key: "driverVersion", label: "DriverVersion", type: "text" },
            { key: "mode", label: "Mode", type: "text" },
            { key: "settings", label: "Settings", type: "textarea", defaultValue: "{}" }
        ]
    },
    {
        id: "tests",
        title: "Tests",
        endpoint: "/api/list-tests",
        createEndpoint: "/api/create-test",
        deleteEndpoint: "/api/delete-test",
        rootField: "tests",
        columns: [
            { key: "id", label: "Id" },
            { key: "name", label: "Name" },
            { key: "description", label: "Description" },
            { key: "iconPath", label: "IconPath" }
        ],
        fields: [
            { key: "name", label: "Name", type: "text" },
            { key: "description", label: "Description", type: "text" },
            { key: "iconPath", label: "IconPath", type: "text" }
        ]
    },
    {
        id: "testConfigs",
        title: "Test Configs",
        endpoint: "/api/list-test-configs",
        createEndpoint: "/api/create-test-config",
        deleteEndpoint: "/api/delete-test-config",
        rootField: "testConfigurations",
        columns: [
            { key: "id", label: "Id" },
            { key: "name", label: "Name" },
            { key: "testId", label: "TestId" },
            { key: "settings", label: "Settings" }
        ],
        fields: [
            { key: "name", label: "Name", type: "text" },
            {
                key: "testId",
                label: "Test",
                type: "select",
                options: { endpoint: "/api/list-tests", rootField: "tests", valueField: "id", displayField: "name" }
            },
            { key: "settings", label: "Settings", type: "textarea", defaultValue: "{}" }
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
    addButton.onclick = () => openDetailsAddDialog(definition);

    const removeButton = document.createElement("button");
    removeButton.type = "button";
    removeButton.textContent = "Remove";
    removeButton.className = "danger-button";
    removeButton.disabled = true;
    removeButton.onclick = () => removeDetailsEntity(definition);

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

async function openDetailsAddDialog(definition)
{
    const dialog = document.getElementById("detailsEntityDialog");
    const title = document.getElementById("detailsEntityDialogTitle");
    const fieldsContainer = document.getElementById("detailsEntityDialogFields");
    const error = document.getElementById("detailsEntityDialogError");
    const submitButton = document.getElementById("detailsEntitySubmitBtn");
    const cancelButton = document.getElementById("detailsEntityCancelBtn");

    title.textContent = `Add ${definition.title}`;
    fieldsContainer.innerHTML = "";
    error.textContent = "";
    submitButton.disabled = false;
    submitButton.onclick = () => submitDetailsEntity(definition);
    cancelButton.onclick = closeDetailsEntityDialog;

    for (const field of definition.fields)
    {
        const row = await createDetailsDialogField(field, submitButton);
        fieldsContainer.appendChild(row);
    }

    dialog.style.display = "flex";
}

async function createDetailsDialogField(field, submitButton)
{
    const row = document.createElement("label");
    row.className = "details-dialog-field";

    const label = document.createElement("span");
    label.textContent = field.label;
    row.appendChild(label);

    let input;
    if (field.type === "textarea")
    {
        input = document.createElement("textarea");
        input.value = field.defaultValue || "";
    }
    else if (field.type === "select")
    {
        input = document.createElement("select");
        await fillDetailsSelect(input, field, submitButton);
    }
    else
    {
        input = document.createElement("input");
        input.type = field.type;
        if (field.defaultValue !== undefined)
            input.value = field.defaultValue;
    }

    input.dataset.fieldKey = field.key;
    input.dataset.fieldType = field.type;
    row.appendChild(input);
    return row;
}

async function fillDetailsSelect(select, field, submitButton)
{
    const response = await fetch(field.options.endpoint);
    const data = await response.json();
    const items = data[field.options.rootField] || [];

    if (items.length === 0)
    {
        const option = document.createElement("option");
        option.value = "";
        option.textContent = `No ${field.label} available`;
        select.appendChild(option);
        select.disabled = true;
        submitButton.disabled = true;
        return;
    }

    for (const item of items)
    {
        const option = document.createElement("option");
        option.value = item[field.options.valueField];
        option.textContent = item[field.options.displayField];
        select.appendChild(option);
    }
}

async function submitDetailsEntity(definition)
{
    const dialog = document.getElementById("detailsEntityDialog");
    const error = document.getElementById("detailsEntityDialogError");
    const submitButton = document.getElementById("detailsEntitySubmitBtn");
    const inputs = document.querySelectorAll("#detailsEntityDialogFields input, #detailsEntityDialogFields select, #detailsEntityDialogFields textarea");
    const payload = {};

    error.textContent = "";
    submitButton.disabled = true;

    for (const input of inputs)
    {
        const key = input.dataset.fieldKey;
        const type = input.dataset.fieldType;
        payload[key] = type === "number" || type === "select" ? Number(input.value || 0) : input.value;
    }

    try
    {
        const response = await fetch(definition.createEndpoint, {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify(payload)
        });

        if (!response.ok)
        {
            error.textContent = await readErrorMessage(response);
            submitButton.disabled = false;
            return;
        }

        dialog.style.display = "none";
        await refreshDetailsTable(definition);
    }
    catch (err)
    {
        console.error(err);
        error.textContent = "Failed to create entry.";
        submitButton.disabled = false;
    }
}

function closeDetailsEntityDialog()
{
    document.getElementById("detailsEntityDialog").style.display = "none";
}

async function removeDetailsEntity(definition)
{
    const panel = document.getElementById(`${definition.id}Table`).closest(".details-table-panel");
    const selectedId = panel.dataset.selectedId;

    if (!selectedId)
    {
        alert("Select a row to remove first.");
        return;
    }

    if (!confirm(`Delete selected ${definition.title} entry?`))
        return;

    try
    {
        const response = await fetch(`${definition.deleteEndpoint}/${selectedId}`, { method: "DELETE" });
        if (!response.ok)
        {
            alert(await readErrorMessage(response));
            return;
        }

        await refreshDetailsTable(definition);
    }
    catch (err)
    {
        console.error(err);
        alert("Failed to delete entry.");
    }
}

async function readErrorMessage(response)
{
    const prefix = `Request failed (${response.status})`;
    const text = await response.text();
    if (!text)
        return prefix;

    try
    {
        const data = JSON.parse(text);
        if (Array.isArray(data.errors) && data.errors.length > 0)
            return `${prefix}: ${data.errors.join("\n")}`;
        return `${prefix}: ${data.message || text}`;
    }
    catch
    {
        return `${prefix}: ${text}`;
    }
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

    try
    {
        const result = await uploadFiles([file]);
        await showImportPlanDialog(result);
    }
    catch (err)
    {
        console.error(err);
        showImportErrorDialog(err.message || "Failed to import file.");
    }
    finally
    {
        event.target.value = "";
    }
}

function showImportResultDialog(result)
{
    const fileInfo = result.files && result.files.length > 0 ? result.files[0] : null;
    if (!fileInfo || fileInfo.format === "unknown")
    {
        showImportErrorDialog("The file was not recognized as a supported benchmark file");
        return;
    }

    const content = getImportResultContent();
    content.innerHTML = "";
    content.classList.remove("import-error");

    content.appendChild(createImportSummaryRow("File", fileInfo.name || "-"));
    content.appendChild(createImportSummaryRow("Format", fileInfo.format || "-"));
    content.appendChild(createImportSummaryRow("Size", fileInfo.size !== undefined ? `${fileInfo.size} bytes` : "-"));
    content.appendChild(createImportJsonSection("Parsed result", fileInfo.parsedData || {}));
    content.appendChild(createImportJsonSection("Normalized result", fileInfo.actionData || {}));

    document.getElementById("importResultDialogTitle").textContent = "Import result";
    document.getElementById("importResultDialog").style.display = "flex";
}

function showImportErrorDialog(message)
{
    const content = getImportResultContent();
    content.innerHTML = "";
    content.classList.add("import-error");
    content.textContent = message;

    document.getElementById("importResultDialogTitle").textContent = "Import result";
    document.getElementById("importResultDialog").style.display = "flex";
}

function getImportResultContent()
{
    return document.getElementById("importResultDialogContent");
}

function createImportSummaryRow(label, value)
{
    const row = document.createElement("div");
    row.className = "import-summary-row";

    const labelElement = document.createElement("span");
    labelElement.textContent = label;

    const valueElement = document.createElement("strong");
    valueElement.textContent = String(value);

    row.appendChild(labelElement);
    row.appendChild(valueElement);
    return row;
}

function createImportJsonSection(title, data)
{
    const section = document.createElement("section");
    section.className = "import-json-section";

    const heading = document.createElement("h4");
    heading.textContent = title;

    const pre = document.createElement("pre");
    pre.textContent = JSON.stringify(data, null, 2);

    section.appendChild(heading);
    section.appendChild(pre);
    return section;
}

function closeImportResultDialog()
{
    document.getElementById("importResultDialog").style.display = "none";
}

// ---------------------------------------------------------------------------
// Import plan dialog
//
// Presents the normalizer's execution plan (result.files[0].actionData) as an
// editable form. Each entity is a row: on the left the reuse/create action
// chooser (+ combobox of existing entities), on the right the entity fields.
// Reuses the metadata in detailsTableDefinitions for field/list definitions.
// ---------------------------------------------------------------------------

// Ordered mapping: normalizer plan key -> detailsTableDefinitions id.
// Parent foreign-key fields are hidden; linkage is implied by the parent row.
const importPlanSections = [
    { planKey: "machine",             definitionId: "machines",             parentKey: null },
    { planKey: "hardwareconfig",      definitionId: "hardwareConfigs",      parentKey: "machineId" },
    { planKey: "softwareenvironment", definitionId: "softwareEnvironments", parentKey: null },
    { planKey: "softwareconfig",      definitionId: "softwareConfigs",      parentKey: "softwareEnvironmentId" },
    { planKey: "test",                definitionId: "tests",                parentKey: null },
    { planKey: "testconfig",          definitionId: "testConfigs",          parentKey: "testId" }
];

// Stashed data from the normalizer, set when the dialog opens and read on submit.
let importPlanStash = { benchmarkrun: {}, sourceFile: "" };

// NOT NULL fields per entity that must be non-empty when creating.
const importPlanRequiredFields = {
    machine:             ["name"],
    hardwareconfig:      ["name"],
    softwareenvironment: ["name", "os", "driverFamily"],
    softwareconfig:      ["name", "driverVersion"],
    test:                ["name"],
    testconfig:          ["name"]
};

function getDefinitionById(id)
{
    return detailsTableDefinitions.find(d => d.id === id);
}

async function showImportPlanDialog(result)
{
    const fileInfo = result.files && result.files.length > 0 ? result.files[0] : null;
    if (!fileInfo || fileInfo.format === "unknown")
    {
        showImportErrorDialog("The file was not recognized as a supported benchmark file");
        return;
    }

    const actionData = fileInfo.actionData || {};

    // Stash data needed at submit time.
    importPlanStash = {
        benchmarkrun: actionData.benchmarkrun || {},
        sourceFile: fileInfo.name || ""
    };

    const body = document.getElementById("importPlanBody");
    const error = document.getElementById("importPlanError");

    body.innerHTML = "";
    error.textContent = "";

    const itemsBySection = await loadImportPlanLists();

    for (const section of importPlanSections)
    {
        const definition = getDefinitionById(section.definitionId);
        const subPlan = actionData[section.planKey] || {};
        const items = itemsBySection[section.definitionId] || [];
        body.appendChild(createImportPlanRow(section, definition, subPlan, items));
    }

    const confirmBtn = document.getElementById("importPlanConfirmBtn");
    confirmBtn.disabled = false;
    confirmBtn.onclick = submitImportPlan;

    document.getElementById("importPlanDialogTitle").textContent = `Import - ${fileInfo.name || "file"}`;
    document.getElementById("importPlanDialog").style.display = "flex";
}

async function loadImportPlanLists()
{
    const entries = await Promise.all(importPlanSections.map(async section =>
    {
        const definition = getDefinitionById(section.definitionId);
        try
        {
            const response = await fetch(definition.endpoint);
            const data = await response.json();
            return [section.definitionId, data[definition.rootField] || []];
        }
        catch (err)
        {
            console.error(`Failed to load ${definition.title}`, err);
            return [section.definitionId, []];
        }
    }));

    return Object.fromEntries(entries);
}

function createImportPlanRow(section, definition, subPlan, items)
{
    const row = document.createElement("section");
    row.className = "import-plan-row";
    row.dataset.planKey = section.planKey;

    const isReuse = subPlan.action === "reuse";

    // ---- Left: action chooser --------------------------------------------
    const actions = document.createElement("div");
    actions.className = "import-plan-actions";

    const heading = document.createElement("h4");
    heading.textContent = definition.title;
    actions.appendChild(heading);

    const radioName = `plan-action-${section.planKey}`;

    const reuseLabel = document.createElement("label");
    const reuseRadio = document.createElement("input");
    reuseRadio.type = "radio";
    reuseRadio.name = radioName;
    reuseRadio.value = "reuse";
    reuseRadio.checked = isReuse;
    reuseRadio.disabled = items.length === 0;
    reuseLabel.appendChild(reuseRadio);
    reuseLabel.appendChild(document.createTextNode(items.length === 0 ? " Reuse (none available)" : " Reuse"));

    const combo = document.createElement("select");
    combo.className = "import-plan-combo";
    for (const item of items)
    {
        const option = document.createElement("option");
        option.value = item.id;
        option.textContent = item.name || `#${item.id}`;
        combo.appendChild(option);
    }
    if (isReuse && subPlan.id !== undefined)
        combo.value = subPlan.id;

    const createLabel = document.createElement("label");
    const createRadio = document.createElement("input");
    createRadio.type = "radio";
    createRadio.name = radioName;
    createRadio.value = "create";
    createRadio.checked = !isReuse;
    createLabel.appendChild(createRadio);
    createLabel.appendChild(document.createTextNode(" Create"));

    actions.appendChild(reuseLabel);
    actions.appendChild(combo);
    actions.appendChild(createLabel);

    // ---- Right: entity fields --------------------------------------------
    const fieldsWrap = document.createElement("div");
    fieldsWrap.className = "import-plan-fields";

    const visibleFields = definition.fields.filter(f => f.key !== section.parentKey);
    const required = importPlanRequiredFields[section.planKey] || [];
    for (const field of visibleFields)
    {
        fieldsWrap.appendChild(createImportPlanField(field, required.includes(field.key)));
    }

    // ---- Wiring ----------------------------------------------------------
    // Values proposed by the normalizer when creating a new entity. Restored
    // whenever the user switches (back) to Create.
    const createDefaults = subPlan.data || {};

    const showSelectedItem = () =>
    {
        const selected = items.find(it => String(it.id) === String(combo.value));
        if (selected)
            populateImportPlanFields(fieldsWrap, selected);
    };

    const applyState = () =>
    {
        const reuse = reuseRadio.checked;
        combo.disabled = !reuse;
        setImportPlanFieldsReadOnly(fieldsWrap, reuse);

        if (reuse)
            showSelectedItem();
        else
            fillImportPlanFields(fieldsWrap, createDefaults);
    };

    reuseRadio.onchange = applyState;
    createRadio.onchange = applyState;
    combo.onchange = () =>
    {
        if (reuseRadio.checked)
            showSelectedItem();
    };

    applyState();

    row.appendChild(actions);
    row.appendChild(fieldsWrap);
    return row;
}

function createImportPlanField(field, isRequired = false)
{
    const wrap = document.createElement("label");
    wrap.className = "import-plan-field";

    const label = document.createElement("span");
    label.textContent = isRequired ? `${field.label} *` : field.label;
    wrap.appendChild(label);

    let input;
    if (field.type === "textarea")
    {
        input = document.createElement("textarea");
    }
    else
    {
        input = document.createElement("input");
        input.type = field.type === "number" ? "number" : "text";
    }

    input.dataset.fieldKey = field.key;
    input.dataset.fieldType = field.type;
    wrap.appendChild(input);
    return wrap;
}

function importPlanFieldInputs(fieldsWrap)
{
    return fieldsWrap.querySelectorAll("input, textarea");
}

function fillImportPlanFields(fieldsWrap, data)
{
    for (const input of importPlanFieldInputs(fieldsWrap))
    {
        const key = input.dataset.fieldKey;
        const value = data[key];
        input.value = value === undefined || value === null ? "" : value;
    }
}

function populateImportPlanFields(fieldsWrap, item)
{
    // List rows expose the same field keys as the create data, so we can
    // fill straight from the selected existing entity.
    fillImportPlanFields(fieldsWrap, item);
}

function setImportPlanFieldsReadOnly(fieldsWrap, readOnly)
{
    for (const input of importPlanFieldInputs(fieldsWrap))
    {
        input.readOnly = readOnly;
        input.classList.toggle("readonly", readOnly);
    }
}

function validateImportPlan()
{
    const errors = [];
    const rows = document.querySelectorAll("#importPlanBody .import-plan-row");

    for (const row of rows)
    {
        const planKey = row.dataset.planKey;
        const action = row.querySelector(`input[name="plan-action-${planKey}"]:checked`);
        if (!action || action.value !== "create")
            continue;

        const required = importPlanRequiredFields[planKey] || [];
        const fieldsWrap = row.querySelector(".import-plan-fields");

        for (const fieldKey of required)
        {
            const input = fieldsWrap.querySelector(`[data-field-key="${fieldKey}"]`);
            if (!input || !input.value.trim())
            {
                const section = importPlanSections.find(s => s.planKey === planKey);
                const def = getDefinitionById(section.definitionId);
                const fieldDef = def.fields.find(f => f.key === fieldKey);
                const label = fieldDef ? fieldDef.label : fieldKey;
                errors.push(`${def.title}: ${label} is required`);
            }
        }
    }

    return errors;
}

function buildImportPlanPayload()
{
    const payload = {};
    const rows = document.querySelectorAll("#importPlanBody .import-plan-row");

    for (const row of rows)
    {
        const planKey = row.dataset.planKey;
        const action = row.querySelector(`input[name="plan-action-${planKey}"]:checked`);

        if (action && action.value === "reuse")
        {
            const combo = row.querySelector(".import-plan-combo");
            payload[planKey] = { action: "reuse", id: Number(combo.value) };
        }
        else
        {
            const fieldsWrap = row.querySelector(".import-plan-fields");
            const data = {};

            for (const input of importPlanFieldInputs(fieldsWrap))
            {
                const key = input.dataset.fieldKey;
                const type = input.dataset.fieldType;
                data[key] = type === "number" ? Number(input.value || 0) : input.value;
            }

            payload[planKey] = { action: "create", data };
        }
    }

    // Attach benchmark run data from stash.
    const run = importPlanStash.benchmarkrun;
    payload.benchmarkrun = {
        timestamp: run.data ? run.data.timestamp || "" : "",
        result: run.data ? run.data.result || {} : {},
        origin: {
            externalId: run.data ? (run.data.origin || {}).externalId || "" : "",
            sourceFile: importPlanStash.sourceFile
        }
    };

    return payload;
}

async function submitImportPlan()
{
    const errorEl = document.getElementById("importPlanError");
    const confirmBtn = document.getElementById("importPlanConfirmBtn");
    errorEl.textContent = "";

    // Client-side validation.
    const validationErrors = validateImportPlan();
    if (validationErrors.length > 0)
    {
        errorEl.textContent = validationErrors.join("\n");
        return;
    }

    confirmBtn.disabled = true;

    try
    {
        const payload = buildImportPlanPayload();
        const response = await fetch("/api/import/execute", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify(payload)
        });

        if (!response.ok)
        {
            errorEl.textContent = await readErrorMessage(response);
            confirmBtn.disabled = false;
            return;
        }

        closeImportPlanDialog();
        await loadRuns();
    }
    catch (err)
    {
        console.error(err);
        errorEl.textContent = "Failed to execute import plan.";
        confirmBtn.disabled = false;
    }
}

function closeImportPlanDialog()
{
    document.getElementById("importPlanDialog").style.display = "none";
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

    if (!res.ok)
        throw new Error(await readErrorMessage(res));

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
window.closeImportResultDialog = closeImportResultDialog;
window.closeImportPlanDialog = closeImportPlanDialog;
window.submitImportPlan = submitImportPlan;
