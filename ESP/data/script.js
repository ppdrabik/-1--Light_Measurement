var progress = document.querySelector(".progressbar");
var range = 182.041; // 65 535 lux/360st = 182.041
var jsonesp;
var datatable = [];
var start = 0;

socket = new WebSocket("ws:/" + "/" + location.host + ":81");
socket.onopen = function (event) {  };
socket.onerror = function (event) { };
socket.onmessage = function (event) {
  if( start == 0){
  jsonesp = JSON.parse(event.data);
  let data = { //tworzymy obiekt
  name: jsonesp.nazwa,
  val: jsonesp.val,
  }
  datatable.push(data); // dodajemy obiekt measurment do tablicy z measurmentem
  createTable(datatable);
  }
  if( start == 1){
  var datafromesp = JSON.parse(event.data);
  console.log(datafromesp);
  document.getElementById("value").innerHTML = datafromesp;
  progress.style.background = `conic-gradient(white, orange ${datafromesp / range}deg, white 0deg)`;
  }
};


const openModalButton = document.getElementById('save') 
const submitModalButton = document.getElementById('submit')
const closeModalButton = document.getElementById('close')
const clearLastData = document.getElementById('clear')
const startButton = document.getElementById('start')
const opacityModal = document.getElementById('modal-opacity')
const table = document.getElementById('tablica')

openModalButton.addEventListener('click', () =>{
  if(table.rows.length<8){
  openModal(modal);
}
})

submitModalButton.addEventListener('click', () =>{
  closeModal(modal);
})

closeModalButton.addEventListener('click', () =>{
  closeModal(modal);
})

clearLastData.addEventListener('click', () =>{
  clearData();
})

startButton.addEventListener('click', () =>{
  start=1;
  socket.send("start");
})

function openModal(modal) {
  modal.classList.add('active')
  opacityModal.classList.add('active')
}

function closeModal(modal) {
  modal.classList.remove('active')
  opacityModal.classList.remove('active')
}

function clearData() {
  socket.send("clear");
  var rowCount = table.rows.length;
  if(table.rows.length>1){
  table.deleteRow(rowCount -1);
  datatable.splice(-1);
}
}

function createTable(dataArray){
  const tableData = dataArray.map(value => {
    return (
      `<tr>
       <td>${value.name}</td>
       <td>${value.val}</td>
    </tr>`
    );
  }).join('');
  const tableBody = document.querySelector("#myTable");
  tableBody.innerHTML = tableData;
}

function addMeasurment(ev){
  ev.preventDefault();  //nie rob tego co zawsze, strona wtedy sie nie odswieza
  let measurement = { //tworzymy obiekt
    name: document.getElementById('input').value, // tekst wpisywany
    val: document.getElementById("value").innerHTML, // wartosc lux
  }
  datatable.push(measurement); // dodajemy obiekt measurment do tablicy z measurmentem
  socket.send(JSON.stringify(measurement));
  document.forms[0].reset(); // czysci pole do wpisywania
  createTable(datatable);
}

document.addEventListener('DOMContentLoaded', () => {
  document.getElementById('submit').addEventListener('click', addMeasurment);

});
