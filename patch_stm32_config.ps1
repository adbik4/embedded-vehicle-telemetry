# Patch STM32 project configuration files to add the missing Nucleo board define,
# update the BSP button/LED/COM usage to the current API, fix the EXTI handler,
# and ensure the CMake link line uses PRIVATE for MX_LINK_LIBS.

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $MyInvocation.MyCommand.Path

function Update-File {
    param(
        [Parameter(Mandatory = $true)][string]$RelativePath,
        [Parameter(Mandatory = $true)][scriptblock]$Transform
    )

    $path = Join-Path $root $RelativePath
    if (-not (Test-Path $path)) {
        throw "Missing file: $path"
    }

    $original = Get-Content -Path $path -Raw
    $updated = & $Transform $original

    if ($updated -ne $original) {
        Set-Content -Path $path -Value $updated -NoNewline
    }
}

Update-File 'Core/Inc/stm32u3xx_nucleo_conf.h' {
    param($content)
    if ($content -notmatch '#define USE_NUCLEO_144') {
        $content = [regex]::Replace(
            $content,
            '(?m)^#define BSP_BUTTON_USER_IT_PRIORITY\s+15U\s*$',
            '$0' + [Environment]::NewLine + '#define USE_NUCLEO_144                      1U',
            1
        )
    }
    return $content
}

Update-File 'Core/Src/stm32u3xx_it.c' {
    param($content)
    return $content -replace 'HAL_EXTI_IRQHandler\(&H_EXTI_13\);', 'BSP_PB_IRQHandler(BUTTON_USER);'
}

Update-File 'Library/App/app_algobuild.c' {
    param($content)
    $content = $content -replace 'BUTTON_KEY', 'BUTTON_USER'
    $content = $content -replace '\bLED2\b', 'LED_RED'
    return $content
}

Update-File 'Library/Target/com.c' {
    param($content)
    $content = $content -replace '__HAL_DMA_GET_TE_FLAG_INDEX(handle_dma)', 'DMA_FLAG_DTE'
    return $content
}

Update-File 'cmake/stm32cubemx/CMakeLists.txt' {
    param($content)
    if ($content -match 'target_link_libraries\(\$\{CMAKE_PROJECT_NAME\}\s+PRIVATE\s+\$\{MX_LINK_LIBS\}\)') {
        return $content
    }

    return $content -replace 'target_link_libraries\(\$\{CMAKE_PROJECT_NAME\}\s+\$\{MX_LINK_LIBS\}\)', 'target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${MX_LINK_LIBS})'
}

Write-Host 'Applied STM32 project compatibility fixes.'
