/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';
import power from '@ohos.power';

@Extend(Button)
function customizeButton() {
  .backgroundColor(Color.Transparent)
  .fontColor($r('app.color.button_text_color'))
}

@CustomDialog
struct PowerCustomDialog {
  controller?: CustomDialogController
  cancel: () => void = () => {
  }
  shutdown: () => void = () => {
  }
  reboot: () => void = () => {
  }

  build() {
    Column() {
      Column() {
        Row() {
          Text($r('app.string.TEXT_POWER_OPTIONS_POWERDIALOG'))
            .fontColor($r('app.color.title_color'))
        }
      }
      Row() {
        Button($r('app.string.BUTTON_CANCEL_POWERDIALOG'))
          .onClick(() => {
            if (this.controller) {
              this.controller.close()
            }
            this.cancel();
          })
          .customizeButton()
        Text('|')
          .fontColor($r('app.color.split_color'))
        Button($r('app.string.BUTTON_REBOOT_POWERDIALOG'))
          .onClick(() => {
            this.reboot()
          })
          .customizeButton()
        Text('|')
          .fontColor($r('app.color.split_color'))
        Button($r('app.string.BUTTON_SHUTDOWN_POWERDIALOG'))
          .onClick(() => {
            this.shutdown()
          })
          .customizeButton()
      }
    }
  }
}

@Entry
@Component
struct PowerDialog {
  dialogController: CustomDialogController = new CustomDialogController({
    builder: PowerCustomDialog({
      cancel: this.onCancel,
      shutdown: () => { this.onShutdown() },
      reboot: () => { this.onReboot() }
    }),
    cancel: this.existApp,
    autoCancel: false,
    alignment: DialogAlignment.Center,
    offset: { dx: 0, dy: -20 },
    gridCount: 4,
    customStyle: false
  });

  timeoutId?: number;
  timeoutMs: number = 20;

  aboutToDisappear() {
    clearTimeout(this.timeoutId);
  }

  onCancel() {
    try {
      console.log('power dialog terminateSelf');
      let storage = LocalStorage.GetShared()
      let session = storage.get<UIExtensionContentSession>('session');
      if (session) {
        session.terminateSelf();
      }
    } catch (err) {
      console.log('power dialog cancel failed: ' + JSON.stringify(err));
    }
  }

  onShutdown() {
    try {
      this.timeoutId = setTimeout(this.onCancel, this.timeoutMs);
      power.shutdown('power_dialog');
    } catch (err) {
      console.log('power dialog shutdown failed: ' + JSON.stringify(err));
    }
  }

  onReboot() {
    try {
      this.timeoutId = setTimeout(this.onCancel, this.timeoutMs);
      power.reboot('power_dialog');
    } catch (err) {
      console.log('power dialog reboot failed: ' + JSON.stringify(err));
    }
  }

  existApp() {
    this.onCancel();
  }

  build() {
    Column(this.dialogController.open()) {}
  }
}
