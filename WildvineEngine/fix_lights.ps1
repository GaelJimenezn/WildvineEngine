$path = "Source\SceneGraph\SceneGraph.cpp"
$content = Get-Content -Raw $path
$old = "outScene.directionalLights.push_back(light);"
$new = @"
switch (light.type) {
			case LightType::Point: outScene.pointLights.push_back(light); break;
			case LightType::Spot:  outScene.spotLights.push_back(light);  break;
			case LightType::Rect:  outScene.rectLights.push_back(light);  break;
			default:               outScene.directionalLights.push_back(light); break;
			}
"@
$content = $content.Replace($old, $new)
Set-Content -Path $path -Value $content -NoNewline
Write-Host "Done"
