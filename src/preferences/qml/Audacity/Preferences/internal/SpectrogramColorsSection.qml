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
            text: qsTrc("spectrogram/settings", "Combine stereo channels by balance")
            checked: settingsModel.stereoCombined

            navigation.panel: root.navigation
            navigation.order: repeater.count + 1
            navigation.name: "ColorsCombinedStereoCheckBox"

            onClicked: settingsModel.stereoCombined = checked
        }

        IncrementalPropertyControlWithTitle {
            width: parent.width
            visible: combinedStereoCheckBox.checked

            navigation.panel: root.navigation
            navigation.order: repeater.count + 2
            navigation.name: "ColorsStereoBalanceGainControl"

            spacing: root.narrowSpacing
            controlWidth: root.mediumControlWidth
            title: qsTrc("spectrogram/settings", "Balance contrast")
            minValue: 0.25
            maxValue: 12
            decimals: 2
            step: 0.25
            currentValue: settingsModel.stereoBalanceGain
            onValueEditingFinished: function (value) {
                settingsModel.stereoBalanceGain = value
            }
        }

        StyledTextLabel {
            width: parent.width
            visible: combinedStereoCheckBox.checked
            text: qsTrc("spectrogram/settings", "Brightness shows mean power. Red: left, purple: equal, blue: right.")
            wrapMode: Text.WordWrap
            color: ui.theme.fontSecondaryColor
        }
    }
}
