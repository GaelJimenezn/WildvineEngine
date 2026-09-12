$headers = @(
  "Include\Actor.h",
  "Include\BaseApp.h",
  "Include\Buffer.h",
  "Include\CommandManager.h",
  "Include\DepthStencilState.h",
  "Include\DepthStencilView.h",
  "Include\Device.h",
  "Include\DeviceContext.h",
  "Include\GUI.h",
  "Include\InputLayout.h",
  "Include\IResource.h",
  "Include\Logger.h",
  "Include\MeshComponent.h",
  "Include\Model3D.h",
  "Include\ModelLoader.h",
  "Include\ParserOBJ.h",
  "Include\Prerequisites.h",
  "Include\RasterizerState.h",
  "Include\RenderTargetView.h",
  "Include\Resource.h",
  "Include\ResourceManager.h",
  "Include\SamplerState.h",
  "Include\ShaderProgram.h",
  "Include\SwapChain.h",
  "Include\Texture.h",
  "Include\Viewport.h",
  "Include\Window.h",
  "Include\ECS\Actor.h",
  "Include\ECS\Component.h",
  "Include\ECS\Entity.h",
  "Include\ECS\LightComponent.h",
  "Include\ECS\MeshRendererComponent.h",
  "Include\ECS\Transform.h",
  "Include\Rendering\DeferredRenderer.h",
  "Include\Rendering\ForwardRenderer.h",
  "Include\Rendering\ISceneRenderer.h",
  "Include\Rendering\Material.h",
  "Include\Rendering\MaterialInstance.h",
  "Include\Rendering\Mesh.h",
  "Include\Rendering\RenderPipeline.h",
  "Include\Rendering\RenderScene.h",
  "Include\Rendering\RenderTypes.h",
  "Include\SceneGraph\HierarchyComponent.h",
  "Include\SceneGraph\SceneGraph.h"
)

$results = @()

foreach ($file in $headers) {
    if (-not (Test-Path $file)) { continue }
    $lines = Get-Content $file
    $total = $lines.Count
    # Count lines with doxygen-style comments
    $doxyLines = ($lines | Where-Object { $_ -match '/\*\*|@brief|@param|@return|@note|///|@file|@class|@enum|@struct|@ingroup' }).Count
    $commentLines = ($lines | Where-Object { $_ -match '//|/\*|\*/' }).Count
    $ratio = if ($total -gt 0) { [math]::Round($doxyLines / $total * 100, 1) } else { 0 }

    $status = if ($doxyLines -eq 0) { "SIN COMENTARIOS" }
              elseif ($ratio -lt 10) { "MUY POCOS" }
              elseif ($ratio -lt 25) { "PARCIAL" }
              else { "OK" }

    $results += [PSCustomObject]@{
        Archivo = $file.Replace("Include\","")
        Lineas  = $total
        DocLines = $doxyLines
        Pct     = "$ratio%"
        Estado  = $status
    }
}

$results | Sort-Object Estado, Pct | Format-Table -AutoSize
