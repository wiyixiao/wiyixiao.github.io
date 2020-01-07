$(document).ready(function(){
	L2Dwidget.init({
		"model": {
			//jsonPath: "https://unpkg.com/live2d-widget-model-shizuku@latest/assets/shizuku.model.json",
			"scale": 1
		},
		"display": {
			"position": "right", 
			"width": 120,  
			"height": 240, 
			"hOffset": 0,
			"vOffset": -20
		},
		"mobile": {
			"show": false,
			"scale": 0.5
		},
		"react": {
			"opacityDefault": 1.0,
			"opacityOnHover": 0.2
		}
	});
	
})
