// Copyright 2020 by John Manfreda. All Rights Reserved.
// https://www.manfreda.org/
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.If not, see<https://www.gnu.org/licenses/>.

using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Input;

namespace GameManagement
{
    /// <summary>
    /// Defines an action that is designated by some set of buttons and/or keys.
    /// 
    /// The way actions work is that you define a set of buttons and keys that trigger the action. You can
    /// then evaluate the action against an InputState which will test to see if any of the buttons or keys
    /// are pressed by a player. You can also set a flag that indicates if the action only occurs once when
    /// the buttons/keys are first pressed or whether the action should occur each frame.
    /// 
    /// Using this InputAction class means that you can configure new actions based on keys and buttons
    /// without having to directly modify the InputState type. This means more customization by your games
    /// without having to change the core classes of Game State Management.
    /// </summary>
    public class InputAction
    {
        private readonly Buttons[] buttons;
        private readonly Keys[] keys;
        private readonly bool newPressOnly;

        // These delegate types map to the methods on InputState. We use these to simplify the evalute method
        // by allowing us to map the appropriate delegates and invoke them, rather than having two separate code paths.
        private delegate bool ButtonPress(Buttons button, PlayerIndex? controllingPlayer, out PlayerIndex player);
        private delegate bool KeyPress(Keys key, PlayerIndex? controllingPlayer, out PlayerIndex player);

        /// <summary>
        /// Initializes a new InputAction.
        /// </summary>
        /// <param name="buttons">An array of buttons that can trigger the action.</param>
        /// <param name="keys">An array of keys that can trigger the action.</param>
        /// <param name="newPressOnly">Whether the action only occurs on the first press of one of the buttons/keys, 
        /// false if it occurs each frame one of the buttons/keys is down.</param>
        public InputAction(Buttons[]? buttons, Keys[]? keys, bool newPressOnly)
        {
            // Store the buttons and keys. If the arrays are null, we create a 0 length array so we don't
            // have to do null checks in the Evaluate method
            this.buttons = buttons?.Clone() as Buttons[] ?? new Buttons[0];
            this.keys = keys?.Clone() as Keys[] ?? new Keys[0];

            this.newPressOnly = newPressOnly;
        }

        /// <summary>
        /// Evaluates the action against a given InputState.
        /// </summary>
        /// <param name="state">The InputState to test for the action.</param>
        /// <param name="controllingPlayer">The player to test, or null to allow any player.</param>
        /// <param name="player">If controllingPlayer is null, this is the player that performed the action.</param>
        /// <returns>True if the action occurred, false otherwise.</returns>
        public bool Evaluate(InputState state, PlayerIndex? controllingPlayer, out PlayerIndex player)
        {
            // Figure out which delegate methods to map from the state which takes care of our "newPressOnly" logic
            ButtonPress buttonTest;
            KeyPress keyTest;
            if (newPressOnly)
            {
                buttonTest = state.IsNewButtonPress;
                keyTest = state.IsNewKeyPress;
            }
            else
            {
                buttonTest = state.IsButtonPressed;
                keyTest = state.IsKeyPressed;
            }

            // Now we simply need to invoke the appropriate methods for each button and key in our collections
            foreach (Buttons button in buttons)
            {
                if (buttonTest(button, controllingPlayer, out player))
                    return true;
            }
            foreach (Keys key in keys)
            {
                if (keyTest(key, controllingPlayer, out player))
                    return true;
            }

            // If we got here, the action is not matched
            player = PlayerIndex.One;
            return false;
        }
    }
}
