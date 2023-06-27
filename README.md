# libtomato

Common library for implementing GyrO:Tomato applications in C++

## Signal Definition file example
Description language codes are [ISO 639-1 two-letter codes](https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes)
```
Enums:
Weapon {
	Gun
		en: A Gun
	Shotgun
		en: Even more Gun
		jp: ショットガン
	None
}
	en: Currently equipped Weapon

In:
MoveCamera {
	float x
	float y
		en: Amount of Vertical rotation (in Radians)
}
	en: Moves the Camera
ToggleFlashlight {
	bool on
}

Out:
OnWeaponChanged {
	Weapon weapon
		en: Current Weapon
}
	en: Called when Player's Weapon have changed
```