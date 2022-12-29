/******************************************************************************
ADOBE SYSTEMS INCORPORATED
 Copyright 2013 Adobe Systems Incorporated
 All Rights Reserved.

NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the 
terms of the Adobe license agreement accompanying it.  If you have received this file from a 
source other than Adobe, then your use, modification, or distribution of it requires the prior 
written permission of Adobe.
******************************************************************************/

var resourceManager = undefined;
var rootAnimator = undefined;
var cbk = undefined;
var interval = 1000/24;
var gStage = undefined;

function init(stage, jsonOutputFile, fps)
{
	console.log("CreateJS animation demo");	
	gStage = stage;
	interval = 1000 / fps;
	//TODO - Wait for load for everything else
	//Load the json
	$.get(jsonOutputFile, function(json) {
		data = json;
		console.log(data);
		resourceManager = new ResourceManager(data);
		reset(stage);
		
		play();
	});
}		
	
function play() 
{
	if(cbk === undefined) 
	{
		cbk = setTimeout(loop, interval);
	}
}

function pause() 
{
	if(cbk !== undefined) 
	{
		clearTimeout(cbk);
		cbk = undefined;
	}			
}

function stop(stage) 
{
	pause();
	reset(stage);
}
	
function loop() 
{

	rootAnimator.play(resourceManager);
	gStage.update();
	//TODO - handle movie clip transforms
	cbk = setTimeout(loop, interval);
}	
	
function reset(stage) 
{
	if(rootAnimator !== undefined) 
	{
		rootAnimator.dispose();
		delete rootAnimator;
	}
	//Getting the index of the last element of the Timeline array
	var maintimelineIndex = resourceManager.m_data.DOMDocument.Timeline.length - 1;
	MainTimeline = resourceManager.m_data.DOMDocument.Timeline[maintimelineIndex]
	//rootAnimator = new TimelineAnimator(stage, resourceManager.m_data.DOMDocument.MainTimeline[0]);
	rootAnimator = new TimelineAnimator(stage, MainTimeline);
	//play();
}
