function setView(view)
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
       loadMachines();
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
      list.parentElement.style.display = "none";
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

