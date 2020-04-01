var map;
var markers = [];
var polylines = [];
function init() {
	map = L.map('mapid').setView([51.509, -0.08], 13);
    L.tileLayer('http://{s}.tile.osm.org/{z}/{x}/{y}.png', 
    {
        attribution: '&copy; <a href="http://osm.org/copyright">OpenStreetMap</a> contributors'
    }).addTo(map);
}

function set_center(latitude, longitude) {
    map.panTo(new L.LatLng(latitude, longitude));
}

function get_center() {
    return map.getCenter();
}

function set_zoom(zoom) {
    map.setZoom(zoom);
}

function add_marker(key, latitude, longitude, parameters) {
    if (key in markers) {
        delete_marker(key);
    }

    if ("icon" in parameters) {
       	parameters["icon"] = new L.Icon({
            iconUrl: parameters["icon"],
            iconAnchor: new L.Point(16, 16)
        });
    }

    var marker = L.marker([latitude, longitude], parameters).addTo(map);

    markers[key] = marker;
    return key;
}

function delete_marker(key) {
    map.removeLayer(markers[key]);
    delete markers[key];
}

function move_marker(key, latitude, longitude) {
    var newLatLng = new L.LatLng(latitude, longitude);
    markers[key].setLatLng(newLatLng);
}

function get_marker_pos(key) {
    marker = markers[key];
    return [marker.getLatLng().lat, marker.getLatLng().lng];
}

function add_polyline(key, latitude, longitude, parameters) {
    if (key in polylines) {
        delete_marker(key);
    }

	var polyline = L.polyline([[latitude, longitude]], parameters).addTo(map);

    polylines[key] = polyline;
    return key;
}

function delete_polyline(key) {
    map.removeLayer(polylines[key]);
    delete polylines[key];
}

function add_point_to_polyline(key, latitude, longitude) {
	polylines[key].addLatLng([latitude, longitude]);
}

function add_points_to_polyline(key, points) {
	polylines[key].setLatLngs(polylines[key].getLatLngs().concat(points));
}
