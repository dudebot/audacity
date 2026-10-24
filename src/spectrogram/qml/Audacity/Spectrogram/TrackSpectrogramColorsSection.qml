/*
 * Audacity: A Digital Audio Editor
 */
import QtQuick 2.15
import QtQuick.Controls 2.15

import Muse.UiComponents

import Audacity.Spectrogram 1.0

TrackSpectrogramBaseSection {
    id: root

    title: qsTrc("appshell/preferences/spectrogram", "Colors")

    required property TrackSpectrogramSettingsModel settingsModel

    Column {
        id: mainColumn

        spacing: root.narrowSpacing

        Repeater {
            id: repeater

            model: ColorSectionParameterListModel {
                settingsModel: root.settingsModel
                columnWidth: root.prefsColumnWidth
            }

            Row {
                spacing: 0

                StyledTextLabel {
                    width: root.mediumControlWidth
                    anchors.verticalCenter: parent.verticalCenter

                    text: colorControlLabel
                    elide: Text.ElideNone
                    wrapMode: Text.NoWrap
                    horizontalAlignment: Text.AlignLeft
                }

                IncrementalPropertyControl {
                    width: root.mediumControlWidth

                    navigation.panel: root.navigation
                    navigation.order: index
                    navigation.name: "ColorsIncrementalControl_" + index

                    minValue: colorControlMinValue
                    maxValue: colorControlMaxValue
                    measureUnitsSymbol: colorControlUnits
                    decimals: 0
                    step: 1

                    currentValue: colorControlCurrentValue
                    onValueEditingFinished: function (newValue) {
                        colorControlCurrentValue = newValue
                    }
                }
            }
        }

        Row {
            spacing: 0
            visible: !settingsModel.stereoCombined

            StyledTextLabel {
                width: root.mediumControlWidth
                anchors.verticalCenter: parent.verticalCenter

                text: qsTrc("spectrogram/settings", "Scheme")
                elide: Text.ElideNone
                wrapMode: Text.NoWrap
                horizontalAlignment: Text.AlignLeft
            }

            StyledDropdown {
                width: root.largeControlWidth

                navigation.panel: root.navigation
                navigation.name: "ColorsSchemeComboBox"
                navigation.order: repeater.count

                model: settingsModel.colorSchemeNames
                currentIndex: settingsModel.colorScheme
                onActivated: function (index, value) {
                    settingsModel.colorScheme = index
                }
            }
        }

        CheckBox {
            id: combinedStereoCheckBox

            text: qsTrc("spectrogram/settings", "Combine stereo channels")
            checked: settingsModel.stereoCombined

            navigation.panel: root.navigation
            navigation.name: "ColorsCombinedStereoCheckBox"
            navigation.order: repeater.count + 1

            onClicked: settingsModel.stereoCombined = !checked
        }

        Row {
            spacing: 0
            visible: combinedStereoCheckBox.checked

            StyledTextLabel {
                width: root.mediumControlWidth
                anchors.verticalCenter: parent.verticalCenter
                text: qsTrc("spectrogram/settings", "Stereo colors")
                horizontalAlignment: Text.AlignLeft
            }

            StyledDropdown {
                width: root.largeControlWidth
                navigation.panel: root.navigation
                navigation.name: "ColorsStereoPaletteComboBox"
                navigation.order: repeater.count + 2
                model: settingsModel.stereoBalancePaletteNames
                currentIndex: settingsModel.stereoBalancePalette
                onActivated: function (index, value) {
                    settingsModel.stereoBalancePalette = index
                }
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
