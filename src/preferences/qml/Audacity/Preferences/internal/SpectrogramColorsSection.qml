/*
 * Audacity: A Digital Audio Editor
 */
import QtQuick 2.15
import QtQuick.Controls 2.15

import Muse.UiComponents

import Audacity.UiComponents 1.0
import Audacity.Spectrogram 1.0

SpectrogramBaseSection {
    id: root

    title: qsTrc("preferences/spectrogram", "Colors")

    required property AbstractSpectrogramSettingsModel settingsModel

    Column {
        id: mainColumn

        spacing: root.mediumSpacing

        Repeater {
            id: repeater

            model: ColorSectionParameterListModel {
                settingsModel: root.settingsModel
                columnWidth: root.prefsColumnWidth
            }

            IncrementalPropertyControlWithTitle {
                id: control

                navigation.panel: root.navigation
                navigation.order: index
                navigation.name: "ColorsIncrementalControl_" + index

                spacing: root.narrowSpacing
                controlWidth: colorControlWidth
                title: colorControlLabel
                minValue: colorControlMinValue
                maxValue: colorControlMaxValue
                measureUnitsSymbol: colorControlUnits
                decimals: 0
                step: 1

                currentValue: colorControlCurrentValue
                onValueEditingFinished: function (value) {
                    colorControlCurrentValue = value
                }
            }
        }

        ComboBoxWithTitle {
            width: parent.width
            visible: !settingsModel.stereoCombined

            navigation.panel: root.navigation
            navigation.order: repeater.count
            navigation.name: "ColorsSchemeComboBox"

            title: qsTrc("spectrogram/settings", "Scheme")
            spacing: root.narrowSpacing
            columnWidth: root.mediumControlWidth
            model: settingsModel.colorSchemeNames
            currentIndex: settingsModel.colorScheme

            onValueEdited: function (index) {
                settingsModel.colorScheme = index
            }
        }

        CheckBox {
            id: combinedStereoCheckBox
            text: qsTrc("spectrogram/settings", "Combine stereo channels")
            checked: settingsModel.stereoCombined

            navigation.panel: root.navigation
            navigation.order: repeater.count + 1
            navigation.name: "ColorsCombinedStereoCheckBox"

            onClicked: settingsModel.stereoCombined = !checked
        }

        ComboBoxWithTitle {
            width: parent.width
            visible: combinedStereoCheckBox.checked
            navigation.panel: root.navigation
            navigation.order: repeater.count + 2
            navigation.name: "ColorsStereoPaletteComboBox"
            spacing: root.narrowSpacing
            columnWidth: root.mediumControlWidth
            title: qsTrc("spectrogram/settings", "Stereo colors")
            model: settingsModel.stereoBalancePaletteNames
            currentIndex: settingsModel.stereoBalancePalette
            onValueEdited: function (index) {
                settingsModel.stereoBalancePalette = index
            }
        }

        StyledTextLabel {
            width: parent.width
            visible: combinedStereoCheckBox.checked
            text: settingsModel.stereoBalancePalette === 0 ? qsTrc("spectrogram/settings", "Left: red. Right: blue. Equal levels blend to purple.") : settingsModel.stereoBalancePalette === 1 ? qsTrc("spectrogram/settings", "Left: magenta. Right: green. Equal levels blend to gray or white.") : qsTrc("spectrogram/settings", "Left: orange. Right: blue. Equal levels blend to gray or white.")
            wrapMode: Text.WordWrap
            color: ui.theme.fontSecondaryColor
        }
    }
}
