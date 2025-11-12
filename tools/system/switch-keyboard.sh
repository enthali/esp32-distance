#!/bin/bash
# Keyboard Layout Switcher for XFCE4/VNC
# This script allows users to easily switch keyboard layouts

echo "🌍 Keyboard Layout Switcher"
echo "=========================="
echo ""

# Show current layout
current_layout=$(setxkbmap -query | grep layout | awk '{print $2}')
echo "Current layout: $current_layout"
echo ""

echo "Available layouts:"
echo "  1. de (German)"
echo "  2. us (US English)"  
echo "  3. uk (UK English)"
echo "  4. fr (French)"
echo "  5. es (Spanish)"
echo "  6. it (Italian)"
echo "  7. Custom layout code"
echo ""

read -p "Select layout (1-7): " choice

case $choice in
    1)
        setxkbmap de
        echo "✅ Switched to German (DE) keyboard layout"
        ;;
    2)
        setxkbmap us
        echo "✅ Switched to US English keyboard layout"
        ;;
    3)
        setxkbmap gb
        echo "✅ Switched to UK English keyboard layout"
        ;;
    4)
        setxkbmap fr
        echo "✅ Switched to French keyboard layout"
        ;;
    5)
        setxkbmap es
        echo "✅ Switched to Spanish keyboard layout"
        ;;
    6)
        setxkbmap it
        echo "✅ Switched to Italian keyboard layout"
        ;;
    7)
        read -p "Enter layout code (e.g., 'de', 'us', 'fr'): " custom_layout
        if setxkbmap "$custom_layout" 2>/dev/null; then
            echo "✅ Switched to $custom_layout keyboard layout"
        else
            echo "❌ Invalid layout code: $custom_layout"
            exit 1
        fi
        ;;
    *)
        echo "❌ Invalid selection"
        exit 1
        ;;
esac

# Show new layout
new_layout=$(setxkbmap -query | grep layout | awk '{print $2}')
echo "New layout: $new_layout"
echo ""
echo "💡 Tip: You can also use 'setxkbmap [layout]' directly"
echo "    Example: setxkbmap de"