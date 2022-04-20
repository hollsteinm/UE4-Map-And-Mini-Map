# UE4-Map-And-Mini-Map

A UE4 plugin that supports volume based minimap rendering and a pan zoom map implementation, all without baking any assets!

## Usage

There are a few components that need to be put together to get this working fully. The 3 parts are a `AMapSourceVolume`, `USceneCaptureComponentMap`, and `USceneMapComponent`. 

### AMapSourceVolume

The rendering of a map is based on this volume. The volumes can be nested. Having volume's within a volume allows you to create a dynamic mini-mapping experience. This enables features such as seamlessly transitioning from a world mini-map to an indoor mini-map. You can also transition between minimaps based on elevation!

Whenever an `AActor` with a `USceneMapComponent` enters a volume, it will register itself with that volume for viewers of the map for that volume.

### USceneMapComponent

An component that acts as a tag and mini-map data provider (such as icon) to a map source volume. This enables visualizing actors on the map, and makes it explicit as to which actors to track on a minimap. The Content has examples of different types of icons to use for friendly, neutral, and enemy actors with this component.

### USceneCaptureComponentMap

This is the component for the actor that needs to write mini map information and do all the maths. The `AMapSourceVolume` uses it, but there is nothing stopping one from attaching it to other `AActor` class types and using some other method of adding content to the mini-map.

## More Help

For more information feel free to [join our discord](https://discord.gg/bQ47YbF)