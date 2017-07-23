<?php
class GoogleAPI {
	static public function GPSToAddress($coords) {
		$url = sprintf("https://maps.googleapis.com/maps/api/geocode/json?latlng=%f,%f&key=%s",
												$coords['latitude'], $coords['longitude'], GOOGLE_API_KEY);
		$json = WingManUtils::GetURLData($url);
		
		$json = json_decode($json, true);
		if(!count($json['results'])) {
			return null;
		}
		
		if(!isset($json['results'][0]['address_components'])) {
			return null;
		}
		
		$data['street'] = '';
		$data['city'] = '';
		$data['state'] = '';
		$data['country'] = '';
		$data['postalCode'] = '';
		$data['address'] = '';
		
		foreach($json['results'][0]['address_components'] as $key) {
			if(in_array('locality', $key['types'])) {
				$data['city'] = $key['long_name'];
				continue;
			}
			
			if(in_array('administrative_area_level_1', $key['types'])) {
				$data['state'] = $key['long_name'];
				continue;
			}
			
			if(in_array('country', $key['types'])) {
				$data['country'] = $key['long_name'];
				continue;
			}
			
			if(in_array('postal_code', $key['types'])) {
				$data['postalCode'] = $key['long_name'];
				continue;
			}
			
			if(in_array('route', $key['types'])) {
				$data['address'] = $key['long_name'];
				continue;
			}
			
			if(in_array('street_number', $key['types'])) {
				$data['street'] = $key['long_name'];
				continue;
			}
		}
		
		return $data;
	}
}